/**************************************************************************/
/*  steam_friends.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                           EIRTeam.Steamworks                           */
/*                         https://ph.eirteam.moe                         */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román (EIRTeam) & contributors.        */
/*                                                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "steam_friends.h"
#include "scene/resources/image_texture.h"
#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

HashMap<uint64_t, Ref<WeakRef>> HBSteamFriend::friend_cache = HashMap<uint64_t, Ref<WeakRef>>();

void HBSteamFriends::_on_lobby_join_requested(Ref<SteamworksCallbackData> p_callback) {
	const GameLobbyJoinRequested_t *req = p_callback->get_data<GameLobbyJoinRequested_t>();
	// Not sure if ConvertToUint64 is safe when using the flat API...
	emit_signal("lobby_join_requested", HBSteamLobby::from_id(req->m_steamIDLobby.ConvertToUint64()));
}

void HBSteamFriends::_bind_methods() {
	ClassDB::bind_method(D_METHOD("activate_game_overlay_invite_dialog", "lobby"), &HBSteamFriends::activate_game_overlay_invite_dialog);
	ClassDB::bind_method(D_METHOD("activate_game_overlay_to_web_page", "web_page", "modal"), &HBSteamFriends::activate_game_overlay_to_web_page);
	ClassDB::bind_method(D_METHOD("set_rich_presence", "key", "value"), &HBSteamFriends::set_rich_presence);
	ADD_SIGNAL(MethodInfo("lobby_join_requested", PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamLobby")));
}

void HBSteamFriends::init_interface() {
	steam_friends = SteamAPI_SteamFriends();
	SW_ERR_FAIL_COND_MSG(steam_friends == nullptr, "Steamworks: Failed to initialize Steam Friends, something catastrophic must have happened");
	Steamworks::get_singleton()->add_callback(GameLobbyJoinRequested_t::k_iCallback, callable_mp(this, &HBSteamFriends::_on_lobby_join_requested));
}

bool HBSteamFriends::is_valid() const {
	return steam_friends != nullptr;
}

void HBSteamFriends::activate_game_overlay_invite_dialog(Ref<HBSteamLobby> p_lobby) const {
	SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog(steam_friends, p_lobby->get_lobby_id());
}

void HBSteamFriends::activate_game_overlay_to_web_page(const String &p_web_page, bool p_modal) const {
	EActivateGameOverlayToWebPageMode mode = EActivateGameOverlayToWebPageMode::k_EActivateGameOverlayToWebPageMode_Default;
	if (p_modal) {
		mode = EActivateGameOverlayToWebPageMode::k_EActivateGameOverlayToWebPageMode_Modal;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(steam_friends, p_web_page.utf8(), mode);
}

void HBSteamFriends::set_rich_presence(const String &p_key, const String &p_value) {
	SteamAPI_ISteamFriends_SetRichPresence(steam_friends, p_key.utf8().get_data(), p_value.utf8().get_data());
}

ISteamFriends *HBSteamFriends::get_interface() const {
	return steam_friends;
}

Ref<HBSteamFriend> HBSteamFriend::from_steam_id(uint64_t p_steam_id) {
	ERR_FAIL_COND_V_MSG(p_steam_id == 0, Ref<HBSteamFriend>(), "An invalid steam user ID was given.");
	if (friend_cache.has(p_steam_id)) {
		Ref<WeakRef> cached_friend_ref = friend_cache[p_steam_id];
		Ref<HBSteamFriend> cached_friend = cached_friend_ref->get_ref();
		if (cached_friend.is_valid()) {
			return cached_friend;
		}
	}
	Ref<HBSteamFriend> steam_friend;
	steam_friend.instantiate();
	steam_friend->steam_id = p_steam_id;
	Ref<WeakRef> weak_ref;
	weak_ref.instantiate();
	weak_ref->set_ref(steam_friend);
	friend_cache.insert(p_steam_id, weak_ref);
	return steam_friend;
}

uint32_t HBSteamFriend::get_account_id() const {
	// Account id are the lowest 32 bits of the steam ID
	return steam_id & 0xFFFFFFFF;
}

bool HBSteamFriend::request_user_information(bool p_include_avatars) const {
	ISteamFriends *friends = Steamworks::get_singleton()->get_friends()->get_interface();
	return SteamAPI_ISteamFriends_RequestUserInformation(friends, steam_id, !p_include_avatars);
}

HBSteamFriend::HBSteamFriend() {
	Steamworks::get_singleton()->add_callback(PersonaStateChange_t::k_iCallback, callable_mp(this, &HBSteamFriend::_on_persona_state_change));
}

void HBSteamFriend::_on_persona_state_change(Ref<SteamworksCallbackData> p_callback) {
	const PersonaStateChange_t *state_change = p_callback->get_data<PersonaStateChange_t>();
	if (state_change->m_ulSteamID == steam_id) {
		if (state_change->m_nChangeFlags & k_EPersonaChangeAvatar) {
			avatar.unref();
		}
		emit_signal("information_updated");
	}
}

void HBSteamFriend::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_persona_name"), &HBSteamFriend::get_persona_name);
	ClassDB::bind_method(D_METHOD("get_steam_id"), &HBSteamFriend::get_steam_id);
	ClassDB::bind_method(D_METHOD("get_avatar"), &HBSteamFriend::get_avatar);
	ClassDB::bind_method(D_METHOD("request_user_information", "include_avatars"), &HBSteamFriend::request_user_information);
	ClassDB::bind_static_method("HBSteamFriend", D_METHOD("from_steam_id", "steam_id"), &HBSteamFriend::from_steam_id);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "avatar"), "", "get_avatar");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "persona_name"), "", "get_persona_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "steam_id"), "", "get_steam_id");
	ADD_SIGNAL(MethodInfo("information_updated"));
}

String HBSteamFriend::get_persona_name() const {
	return String::utf8(SteamAPI_ISteamFriends_GetFriendPersonaName(Steamworks::get_singleton()->get_friends()->get_interface(), steam_id));
}

Ref<Texture2D> HBSteamFriend::get_avatar() const {
	if (avatar.is_valid()) {
		return avatar;
	}
	ISteamUtils *utils = Steamworks::get_singleton()->get_utils()->get_interface();
	int image_handle = SteamAPI_ISteamFriends_GetMediumFriendAvatar(Steamworks::get_singleton()->get_friends()->get_interface(), steam_id);
	uint32_t width, height;
	SteamAPI_ISteamUtils_GetImageSize(utils, image_handle, &width, &height);
	Vector<uint8_t> image_data;
	image_data.resize(width * height * 4);
	SteamAPI_ISteamUtils_GetImageRGBA(utils, image_handle, image_data.ptrw(), image_data.size());
	Ref<Image> image = Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, image_data);
	if (image.is_valid()) {
		const_cast<HBSteamFriend *>(this)->avatar = ImageTexture::create_from_image(image);
	}
	return avatar;
}
uint64_t HBSteamFriend::get_steam_id() const {
	return steam_id;
}
