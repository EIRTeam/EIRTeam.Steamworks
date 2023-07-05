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
#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

void HBSteamFriends::init_interface() {
	steam_friends = SteamAPI_SteamFriends();
	SW_ERR_FAIL_COND_MSG(steam_friends == nullptr, "Steamworks: Failed to initialize Steam Friends, something catastrophic must have happened");
}

bool HBSteamFriends::is_valid() const {
	return steam_friends != nullptr;
}

ISteamFriends *HBSteamFriends::get_interface() const {
	return steam_friends;
}

Ref<HBSteamFriend> HBSteamFriend::from_steam_id(uint64_t p_steam_id) {
	Ref<HBSteamFriend> steam_friend;
	steam_friend.instantiate();
	steam_friend->steam_id = p_steam_id;
	return steam_friend;
}

void HBSteamFriend::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_persona_name"), &HBSteamFriend::get_persona_name);
	ClassDB::bind_method(D_METHOD("get_steam_id"), &HBSteamFriend::get_steam_id);
	ClassDB::bind_method(D_METHOD("get_avatar"), &HBSteamFriend::get_avatar);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "avatar"), "", "get_avatar");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "persona_name"), "", "get_persona_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "steam_id"), "", "get_steam_id");
}

String HBSteamFriend::get_persona_name() const {
	return SteamAPI_ISteamFriends_GetFriendPersonaName(Steamworks::get_singleton()->get_friends()->get_interface(), steam_id);
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
