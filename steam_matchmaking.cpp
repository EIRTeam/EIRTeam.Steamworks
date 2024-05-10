/**************************************************************************/
/*  steam_matchmaking.cpp                                                 */
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

#include "steam_matchmaking.h"

#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

void HBSteamMatchmaking::_bind_methods() {
	ClassDB::bind_method("create_lobby_list_query", &HBSteamMatchmaking::create_lobby_list_query);
	ClassDB::bind_method("is_valid", &HBSteamMatchmaking::is_valid);
}

void HBSteamMatchmaking::init_interface() {
	steam_matchmaking = SteamAPI_SteamMatchmaking();
	SW_ERR_FAIL_COND_MSG(steam_matchmaking == nullptr, "Steamworks: Failed to initialize Steam Matchmaking, something catastrophic must have happened");
}

bool HBSteamMatchmaking::is_valid() const {
	return steam_matchmaking != nullptr;
}

ISteamMatchmaking *HBSteamMatchmaking::get_interface() const {
	return steam_matchmaking;
}

Ref<HBLobbyListQuery> HBSteamMatchmaking::create_lobby_list_query() {
	Ref<HBLobbyListQuery> list_query;
	list_query.instantiate();
	return list_query;
}

void HBSteamLobby::_join_lobby(uint64_t p_lobby_id) {
	lobby_id = p_lobby_id;
	SteamAPICall_t call = SteamAPI_ISteamMatchmaking_JoinLobby(Steamworks::get_singleton()->get_matchmaking()->get_interface(), p_lobby_id);
	Steamworks::get_singleton()->add_call_result_callback(call, callable_mp(this, &HBSteamLobby::_on_lobby_entered));
}

void HBSteamLobby::_create_lobby(SteamworksConstants::LobbyType p_lobby_type, int p_max_members) {
	SteamAPICall_t call = SteamAPI_ISteamMatchmaking_CreateLobby(Steamworks::get_singleton()->get_matchmaking()->get_interface(), (ELobbyType)p_lobby_type, p_max_members);
	Steamworks::get_singleton()->add_call_result_callback(call, callable_mp(this, &HBSteamLobby::_on_lobby_created));
}

void HBSteamLobby::_on_lobby_entered(Ref<SteamworksCallbackData> p_callback_data) {
	const LobbyEnter_t *lobby_enter = p_callback_data->get_data<LobbyEnter_t>();
	if (lobby_enter->m_ulSteamIDLobby == lobby_id) {
		emit_signal("lobby_entered", lobby_enter->m_EChatRoomEnterResponse);
		Steamworks::get_singleton()->add_callback(LobbyChatMsg_t::k_iCallback, callable_mp(this, &HBSteamLobby::_on_lobby_chat_msg));
	}
}

void HBSteamLobby::_on_lobby_created(Ref<SteamworksCallbackData> p_callback_data, bool p_io_failure) {
	const LobbyCreated_t *lobby_created = p_callback_data->get_data<LobbyCreated_t>();
	lobby_id = lobby_created->m_ulSteamIDLobby;
	emit_signal("lobby_created", (SWC::Result)lobby_created->m_eResult);
}

void HBSteamLobby::_on_lobby_chat_msg(Ref<SteamworksCallbackData> p_callback_data) {
	const LobbyChatMsg_t *msg = p_callback_data->get_data<LobbyChatMsg_t>();
	if (msg->m_ulSteamIDLobby != lobby_id) {
		return;
	}
	Vector<uint8_t> msg_data;
	msg_data.resize(4000);

	uint64_t steam_id_user = msg->m_ulSteamIDUser;
	// This is unused because we already have steam_id_user and because we don't deal with
	// c++ types for cross-compiler compatibility
	uint64_t _steam_id_ret;

	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	EChatEntryType entry_type;
	int bytes_received = SteamAPI_ISteamMatchmaking_GetLobbyChatEntry(mm, lobby_id, msg->m_iChatID, (CSteamID *)&_steam_id_ret, msg_data.ptrw(), msg_data.size(), &entry_type);
	msg_data.resize(bytes_received);

	emit_signal("chat_message_received", HBSteamFriend::from_steam_id(steam_id_user), entry_type, msg_data);
}

void HBSteamLobby::_on_lobby_data_updated(Ref<SteamworksCallbackData> p_callback_data) {
	LobbyDataUpdate_t *update = (LobbyDataUpdate_t *)p_callback_data->get_data<LobbyDataUpdate_t>();
	if (update->m_ulSteamIDLobby != lobby_id) {
		return;
	}
	if (update->m_ulSteamIDMember == lobby_id) {
		emit_signal("lobby_data_updated");
	} else {
		emit_signal("lobby_member_data_updated", HBSteamFriend::from_steam_id(update->m_ulSteamIDMember));
	}
}

void HBSteamLobby::_on_lobby_chat_updated(Ref<SteamworksCallbackData> p_callback_data) {
	LobbyChatUpdate_t *update = (LobbyChatUpdate_t *)p_callback_data->get_data<LobbyChatUpdate_t>();
	if (update->m_ulSteamIDLobby != lobby_id) {
		return;
	}
	// I'm pretty sure kicking and banning doesn't actually work this way anymore
	// so we don't have a explicit signal for that
	int leave_mask = EChatMemberStateChange::k_EChatMemberStateChangeDisconnected | EChatMemberStateChange::k_EChatMemberStateChangeBanned | EChatMemberStateChange::k_EChatMemberStateChangeKicked | EChatMemberStateChange::k_EChatMemberStateChangeLeft;
	if (update->m_rgfChatMemberStateChange & leave_mask) {
		emit_signal("member_left", HBSteamFriend::from_steam_id(update->m_ulSteamIDUserChanged));
	} else if (update->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) {
		emit_signal("member_joined", HBSteamFriend::from_steam_id(update->m_ulSteamIDUserChanged));
	}
	emit_signal("lobby_chat_updated", HBSteamFriend::from_steam_id(update->m_ulSteamIDMakingChange), HBSteamFriend::from_steam_id(update->m_ulSteamIDUserChanged), update->m_rgfChatMemberStateChange);
}

void HBSteamLobby::_bind_methods() {
	ClassDB::bind_method(D_METHOD("join_lobby"), &HBSteamLobby::join_lobby);
	ClassDB::bind_method(D_METHOD("get_owner"), &HBSteamLobby::get_owner);
	ClassDB::bind_method(D_METHOD("set_lobby_owner", "owner"), &HBSteamLobby::set_lobby_owner);
	ClassDB::bind_method(D_METHOD("set_data", "key", "value"), &HBSteamLobby::set_data);
	ClassDB::bind_method(D_METHOD("get_data", "key"), &HBSteamLobby::get_data);
	ClassDB::bind_method(D_METHOD("get_member_data", "member", "key"), &HBSteamLobby::get_member_data);
	ClassDB::bind_method(D_METHOD("get_members"), &HBSteamLobby::get_members);
	ClassDB::bind_method(D_METHOD("get_members_count"), &HBSteamLobby::get_members_count);
	ClassDB::bind_method(D_METHOD("send_chat_string", "message"), &HBSteamLobby::send_chat_string);
	ClassDB::bind_method(D_METHOD("send_chat_binary", "message"), &HBSteamLobby::send_chat_binary);
	ClassDB::bind_method(D_METHOD("set_member_data", "key", "data"), &HBSteamLobby::set_member_data);
	ClassDB::bind_method(D_METHOD("set_lobby_joinable", "joinable"), &HBSteamLobby::set_lobby_joinable);
	ClassDB::bind_method(D_METHOD("leave_lobby"), &HBSteamLobby::leave_lobby);
	ClassDB::bind_method(D_METHOD("get_lobby_id"), &HBSteamLobby::get_lobby_id);
	ClassDB::bind_method(D_METHOD("is_owned_by_local_user"), &HBSteamLobby::is_owned_by_local_user);
	ClassDB::bind_method(D_METHOD("get_all_lobby_data"), &HBSteamLobby::get_all_lobby_data);

	ClassDB::bind_method(D_METHOD("get_max_members"), &HBSteamLobby::get_max_members);
	ClassDB::bind_method(D_METHOD("set_max_members", "max_members"), &HBSteamLobby::set_max_members);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_members"), "set_max_members", "get_max_members");

	ClassDB::bind_static_method("HBSteamLobby", D_METHOD("create_lobby", "lobby_type", "max_members"), &HBSteamLobby::create_lobby);
	ClassDB::bind_static_method("HBSteamLobby", D_METHOD("from_id", "lobby_id"), &HBSteamLobby::from_id);
	ADD_SIGNAL(MethodInfo("lobby_entered", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("lobby_data_updated"));
	ADD_SIGNAL(MethodInfo("lobby_member_data_updated", PropertyInfo(Variant::OBJECT, "member", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
	ADD_SIGNAL(MethodInfo("chat_message_received", PropertyInfo(Variant::OBJECT, "sender", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), PropertyInfo(Variant::INT, "type"), PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data")));
	ADD_SIGNAL(MethodInfo("lobby_chat_updated", PropertyInfo(Variant::OBJECT, "changed", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), PropertyInfo(Variant::OBJECT, "making_change", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), PropertyInfo(Variant::INT, "change")));
	ADD_SIGNAL(MethodInfo("member_joined", PropertyInfo(Variant::OBJECT, "new_member", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
	ADD_SIGNAL(MethodInfo("member_left", PropertyInfo(Variant::OBJECT, "new_member", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "owner", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), "", "get_owner");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lobby_id"), "", "get_lobby_id");
}

void HBSteamLobby::join_lobby() {
	ERR_FAIL_COND_MSG(lobby_id == 0, "Lobby ID is invalid");
	_join_lobby(lobby_id);
}

Ref<HBSteamFriend> HBSteamLobby::get_owner() const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	ERR_FAIL_COND_V_MSG(lobby_id == 0, Ref<HBSteamFriend>(), "Lobby is invalid");
	uint64_t owner = SteamAPI_ISteamMatchmaking_GetLobbyOwner(mm, lobby_id);
	return HBSteamFriend::from_steam_id(owner);
}

bool HBSteamLobby::set_lobby_owner(Ref<HBSteamFriend> p_new_owner) {
	ERR_FAIL_COND_V_MSG(!p_new_owner.is_valid(), false, "New lobby owner was invalid");
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_SetLobbyOwner(mm, lobby_id, p_new_owner->get_steam_id());
}

Ref<HBSteamLobby> HBSteamLobby::create_lobby(SteamworksConstants::LobbyType p_lobby_type, int p_max_members) {
	Ref<HBSteamLobby> lobby;
	lobby.instantiate();
	lobby->_create_lobby(p_lobby_type, p_max_members);
	return lobby;
}

Ref<HBSteamLobby> HBSteamLobby::from_id(uint64_t lobby_id) {
	Ref<HBSteamLobby> lobby;
	lobby.instantiate();
	lobby->lobby_id = lobby_id;
	return lobby;
}

TypedArray<HBSteamFriend> HBSteamLobby::get_members() const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	int lobby_member_count = SteamAPI_ISteamMatchmaking_GetNumLobbyMembers(mm, lobby_id);
	TypedArray<HBSteamFriend> out;
	for (int i = 0; i < lobby_member_count; i++) {
		uint64_t user_steam_id = SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex(mm, lobby_id, i);
		Ref<HBSteamFriend> steam_friend = HBSteamFriend::from_steam_id(user_steam_id);
		if (steam_friend.is_valid()) {
			out.push_back(steam_friend);
		}
	}
	return out;
}

int HBSteamLobby::get_members_count() const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_GetNumLobbyMembers(mm, lobby_id);
}

bool HBSteamLobby::set_data(const String &p_key, const String &p_value) {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_SetLobbyData(mm, lobby_id, p_key.utf8().get_data(), p_value.utf8().get_data());
}

void HBSteamLobby::set_member_data(const String &p_key, const String &p_value) {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	SteamAPI_ISteamMatchmaking_SetLobbyMemberData(mm, lobby_id, p_key.utf8().get_data(), p_value.utf8().get_data());
}

Dictionary HBSteamLobby::get_all_lobby_data() const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	int data_count = SteamAPI_ISteamMatchmaking_GetLobbyDataCount(mm, lobby_id);

	Dictionary out;

	Vector<char> key_buffer;
	key_buffer.resize(k_nMaxLobbyKeyLength);
	Vector<char> data_buffer;
	data_buffer.resize(k_cubChatMetadataMax);

	char *key_buffer_w = key_buffer.ptrw();
	char *data_buffer_w = data_buffer.ptrw();

	for (int i = 0; i < data_count; i++) {
		bool success = SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex(mm, lobby_id, i, key_buffer_w, key_buffer.size(), data_buffer_w, data_buffer.size());
		if (!success) {
			continue;
		}
		String key = String::utf8(key_buffer_w);
		String value = String::utf8(data_buffer_w);
		out[key] = value;
	}
	return out;
}

String HBSteamLobby::get_data(const String &p_key) const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return String::utf8(SteamAPI_ISteamMatchmaking_GetLobbyData(mm, lobby_id, p_key.utf8().get_data()));
}

String HBSteamLobby::get_member_data(const Ref<HBSteamFriend> &p_steam_user, const String &p_key) const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return String::utf8(SteamAPI_ISteamMatchmaking_GetLobbyMemberData(mm, lobby_id, p_steam_user->get_steam_id(), p_key.utf8().get_data()));
}

int HBSteamLobby::get_max_members() const {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit(mm, lobby_id);
}

void HBSteamLobby::set_max_members(int p_max_members) const {
	ERR_FAIL_COND_MSG(get_owner() != Steamworks::get_singleton()->get_user()->get_local_user(), "Only the owner can set the max members in a lobby");
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	ERR_FAIL_COND_MSG(!SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit(mm, lobby_id, p_max_members), "Setting lobby member limit failed");
}

bool HBSteamLobby::send_chat_string(const String &p_chat_string) {
	return send_chat_binary(p_chat_string.to_utf8_buffer());
}

bool HBSteamLobby::send_chat_binary(const PackedByteArray &p_buffer) {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_SendLobbyChatMsg(mm, lobby_id, p_buffer.ptr(), p_buffer.size());
}

bool HBSteamLobby::set_lobby_joinable(bool p_joinable) {
	ERR_FAIL_COND_V_MSG(lobby_id == 0, false, "Lobby ID is invalid");
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	return SteamAPI_ISteamMatchmaking_SetLobbyJoinable(mm, lobby_id, p_joinable);
}

bool HBSteamLobby::is_owned_by_local_user() const {
	return Steamworks::get_singleton()->get_user()->get_local_user() == get_owner();
}

uint64_t HBSteamLobby::get_lobby_id() const {
	return lobby_id;
}

void HBSteamLobby::leave_lobby() {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	SteamAPI_ISteamMatchmaking_LeaveLobby(mm, lobby_id);
	lobby_id = 0;
}

HBSteamLobby::HBSteamLobby() {
	// listen to global LobbyEnter_t callbacks since they might be triggered by lobby creation
	Steamworks::get_singleton()->add_callback(LobbyEnter_t::k_iCallback, callable_mp(this, &HBSteamLobby::_on_lobby_entered));
	Steamworks::get_singleton()->add_callback(LobbyDataUpdate_t::k_iCallback, callable_mp(this, &HBSteamLobby::_on_lobby_data_updated));
	Steamworks::get_singleton()->add_callback(LobbyChatUpdate_t::k_iCallback, callable_mp(this, &HBSteamLobby::_on_lobby_chat_updated));
}

void HBLobbyListQuery::_bind_methods() {
	ClassDB::bind_method("filter_distance_close", &HBLobbyListQuery::filter_distance_close);
	ClassDB::bind_method("filter_distance_far", &HBLobbyListQuery::filter_distance_far);
	ClassDB::bind_method("filter_distance_worldwide", &HBLobbyListQuery::filter_distance_worldwide);
	ClassDB::bind_method(D_METHOD("order_by_near", "key", "value"), &HBLobbyListQuery::order_by_near);
	ClassDB::bind_method(D_METHOD("with_equal", "key", "value"), &HBLobbyListQuery::with_equal);
	ClassDB::bind_method(D_METHOD("with_higher", "key", "value"), &HBLobbyListQuery::with_higher);
	ClassDB::bind_method(D_METHOD("with_key_value", "key", "value"), &HBLobbyListQuery::with_key_value);
	ClassDB::bind_method(D_METHOD("with_lower", "key", "value"), &HBLobbyListQuery::with_lower);
	ClassDB::bind_method(D_METHOD("with_not_equal", "key", "value"), &HBLobbyListQuery::with_not_equal);
	ClassDB::bind_method(D_METHOD("with_slots_available", "min_slots"), &HBLobbyListQuery::with_slots_available);
	ClassDB::bind_method(D_METHOD("with_max_results", "max_results"), &HBLobbyListQuery::with_slots_available);
	ClassDB::bind_method("request_lobby_list", &HBLobbyListQuery::request_lobby_list);

	ADD_SIGNAL(MethodInfo("received_lobby_list", PropertyInfo(Variant::ARRAY, "lobbies", PROPERTY_HINT_ARRAY_TYPE, "HBSteamLobby")));
}

void HBLobbyListQuery::_add_numerical_filter(const String &p_key, int p_value, SWC::LobbyComparison p_comparison) {
	ERR_FAIL_COND_MSG(p_key.is_empty(), "Filter key must not be empty");
	ERR_FAIL_COND_MSG(p_key.length() > k_nMaxLobbyKeyLength, vformat("Filter key must not be longer than %d characters.", k_nMaxLobbyKeyLength));

	NumericalFilter filter;
	filter.comparison = p_comparison;
	filter.value = p_value;
	filter.key = p_key;

	numerical_filters.push_back(filter);
}

void HBLobbyListQuery::_on_lobby_list_received(Ref<SteamworksCallbackData> p_callback_data, bool p_io_falure) {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	const LobbyMatchList_t *lobby_list_info = p_callback_data->get_data<LobbyMatchList_t>();
	TypedArray<HBSteamLobby> lobbies;
	for (int i = 0; i < (int)lobby_list_info->m_nLobbiesMatching; i++) {
		uint64_t lobby_id = SteamAPI_ISteamMatchmaking_GetLobbyByIndex(mm, i);
		if (lobby_id == 0) {
			continue;
		}
		lobbies.push_back(HBSteamLobby::from_id(lobby_id));
	}

	emit_signal("received_lobby_list", lobbies);
}

Ref<HBLobbyListQuery> HBLobbyListQuery::filter_distance_close() {
	distance_filter = SWC::LOBBY_DISTANCE_FILTER_CLOSE;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::filter_distance_far() {
	distance_filter = SWC::LOBBY_DISTANCE_FILTER_FAR;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::filter_distance_worldwide() {
	distance_filter = SWC::LOBBY_DISTANCE_FILTER_WORLDWIDE;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::order_by_near(const String &p_key, int p_value) {
	ERR_FAIL_COND_V_MSG(p_key.is_empty(), this, "Filter key must not be empty");
	ERR_FAIL_COND_V_MSG(p_key.length() > k_nMaxLobbyKeyLength, this, vformat("Filter key must not be longer than %d characters.", k_nMaxLobbyKeyLength));
	if (!near_value_filters.has(p_key)) {
		near_value_filters.insert(p_key, Vector<int>());
	}
	near_value_filters[p_key].push_back(p_value);
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_equal(const String &p_key, int p_value) {
	_add_numerical_filter(p_key, p_value, SWC::LOBBY_COMPARISON_EQUAL);
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_higher(const String &p_key, int p_value) {
	_add_numerical_filter(p_key, p_value, SWC::LOBBY_COMPARISON_GREATER_THAN);
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_key_value(const String &p_key, const String &p_value) {
	ERR_FAIL_COND_V_MSG(p_key.is_empty(), this, "Filter key must not be empty");
	ERR_FAIL_COND_V_MSG(p_key.length() > k_nMaxLobbyKeyLength, this, vformat("Filter key must not be longer than %d characters.", k_nMaxLobbyKeyLength));
	ERR_FAIL_COND_V_MSG(p_key.is_empty(), this, "Filter value must not be empty");
	string_filters[p_key] = p_value;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_lower(const String &p_key, int p_value) {
	_add_numerical_filter(p_key, p_value, SWC::LOBBY_COMPARISON_LESS_THAN);
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_max_results(int p_max_results) {
	max_results = p_max_results;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_not_equal(const String &p_key, int p_value) {
	_add_numerical_filter(p_key, p_value, SWC::LOBBY_COMPARISON_NOT_EQUAL);
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::with_slots_available(int p_min_slots) {
	slots_available = p_min_slots;
	return this;
}

Ref<HBLobbyListQuery> HBLobbyListQuery::request_lobby_list() {
	ISteamMatchmaking *mm = Steamworks::get_singleton()->get_matchmaking()->get_interface();
	for (NumericalFilter &filter : numerical_filters) {
		SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter(mm, filter.key.utf8().get_data(), filter.value, (ELobbyComparison)filter.comparison);
	}

	for (KeyValue<String, Vector<int>> &kv : near_value_filters) {
		for (int value : kv.value) {
			SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter(mm, kv.key.utf8().get_data(), value);
		}
	}

	for (KeyValue<String, String> kv : string_filters) {
		SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter(mm, kv.key.utf8().get_data(), kv.value.utf8().get_data(), (ELobbyComparison)SWC::LOBBY_COMPARISON_EQUAL);
	}

	SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter(mm, (ELobbyDistanceFilter)distance_filter);

	if (max_results != -1) {
		SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter(mm, max_results);
	}
	if (slots_available != -1) {
		SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable(mm, slots_available);
	}

	SteamAPICall_t api_call = SteamAPI_ISteamMatchmaking_RequestLobbyList(mm);
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBLobbyListQuery::_on_lobby_list_received));
	return this;
}
