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

void HBSteamLobby::_on_lobby_entered(Ref<SteamworksCallbackData> p_callback_data, bool p_io_failure) {
	const LobbyEnter_t *lobby_enter = p_callback_data->get_data<LobbyEnter_t>();
	if (lobby_enter->m_ulSteamIDLobby == lobby_id) {
		emit_signal("lobby_entered", lobby_enter->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess);
	}
}

void HBSteamLobby::_on_lobby_created(Ref<SteamworksCallbackData> p_callback_data, bool p_io_failure) {
	const LobbyCreated_t *lobby_created = p_callback_data->get_data<LobbyCreated_t>();
	lobby_id = lobby_created->m_ulSteamIDLobby;
	emit_signal("lobby_created", (SWC::Result)lobby_created->m_eResult);
}

void HBSteamLobby::_bind_methods() {
	ClassDB::bind_method(D_METHOD("join_lobby"), &HBSteamLobby::join_lobby);
	ClassDB::bind_static_method("HBSteamLobby", D_METHOD("create_lobby", "lobby_type", "max_members"), &HBSteamLobby::create_lobby);
	ClassDB::bind_static_method("HBSteamLobby", D_METHOD("from_id", "lobby_id"), &HBSteamLobby::from_id);
	ADD_SIGNAL(MethodInfo("lobby_entered", PropertyInfo(Variant::BOOL, "success")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "result")));
}

void HBSteamLobby::join_lobby() {
	ERR_FAIL_COND_MSG(lobby_id == 0, "Lobby ID is invalid");
	_join_lobby(lobby_id);
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

HBSteamLobby::HBSteamLobby() {
	// listen to global LobbyEnter_t callbacks since they might be triggered by lobby creation
	Steamworks::get_singleton()->add_callback(LobbyEnter_t::k_iCallback, callable_mp(this, &HBSteamLobby::_on_lobby_entered).bind(false));
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
	ClassDB::bind_method("request_lobby_list", &HBLobbyListQuery::filter_distance_worldwide);

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
