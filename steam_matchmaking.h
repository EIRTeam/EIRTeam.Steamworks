/**************************************************************************/
/*  steam_matchmaking.h                                                   */
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

#ifndef STEAM_MATCHMAKING_H
#define STEAM_MATCHMAKING_H

#include "core/object/ref_counted.h"
#include "steam_friends.h"
#include "steamworks_callback_data.h"
#include "steamworks_constants.gen.h"

class ISteamMatchmaking;

class HBSteamLobby : public RefCounted {
	GDCLASS(HBSteamLobby, RefCounted);

private:
	uint64_t lobby_id;
	void _join_lobby(uint64_t p_lobby_id);
	void _create_lobby(SteamworksConstants::LobbyType p_lobby_type, int p_max_members);
	void _on_lobby_entered(Ref<SteamworksCallbackData> p_callback_data);
	void _on_lobby_created(Ref<SteamworksCallbackData> p_callback_data, bool p_io_failure);
	void _on_lobby_chat_msg(Ref<SteamworksCallbackData> p_callback_data);
	void _on_lobby_data_updated(Ref<SteamworksCallbackData> p_callback_data);
	void _on_lobby_chat_updated(Ref<SteamworksCallbackData> p_callback_data);

protected:
	static void _bind_methods();

public:
	void join_lobby();
	Ref<HBSteamFriend> get_owner() const;
	bool set_lobby_owner(Ref<HBSteamFriend> p_new_owner);
	static Ref<HBSteamLobby> create_lobby(SteamworksConstants::LobbyType p_lobby_type, int p_max_members);
	static Ref<HBSteamLobby> from_id(uint64_t lobby_id);
	TypedArray<HBSteamFriend> get_members() const;
	int get_members_count() const;
	bool set_data(const String &p_key, const String &p_value);
	void set_member_data(const String &p_key, const String &p_value);
	Dictionary get_all_lobby_data() const;
	String get_data(const String &p_key) const;
	String get_member_data(const Ref<HBSteamFriend> &p_steam_user, const String &p_key) const;
	int get_max_members() const;
	void set_max_members(int p_max_members) const;
	bool send_chat_string(const String &p_chat_string);
	bool send_chat_binary(const PackedByteArray &p_buffer);
	bool set_lobby_joinable(bool p_joinable);
	bool is_owned_by_local_user() const;
	uint64_t get_lobby_id() const;
	String get_lobby_name() const;
	void leave_lobby();
	HBSteamLobby();
};

class HBLobbyListQuery : public RefCounted {
	GDCLASS(HBLobbyListQuery, RefCounted);
	struct NumericalFilter {
		String key;
		int value;
		SWC::LobbyComparison comparison;
	};

	Vector<NumericalFilter> numerical_filters;
	HashMap<String, Vector<int>> near_value_filters;
	HashMap<String, String> string_filters;
	SWC::LobbyDistanceFilter distance_filter = SWC::LobbyDistanceFilter::LOBBY_DISTANCE_FILTER_DEFAULT;
	int max_results = -1;
	int slots_available = -1;

protected:
	static void _bind_methods();

private:
	void _add_numerical_filter(const String &p_key, int p_value, SWC::LobbyComparison p_comparison);
	void _on_lobby_list_received(Ref<SteamworksCallbackData> p_callback_data, bool p_io_falure);

public:
	Ref<HBLobbyListQuery> filter_distance_close();
	Ref<HBLobbyListQuery> filter_distance_far();
	Ref<HBLobbyListQuery> filter_distance_worldwide();
	Ref<HBLobbyListQuery> order_by_near(const String &p_key, int p_value);
	Ref<HBLobbyListQuery> with_equal(const String &p_key, int p_value);
	Ref<HBLobbyListQuery> with_higher(const String &p_key, int p_value);
	Ref<HBLobbyListQuery> with_key_value(const String &p_key, const String &p_value);
	Ref<HBLobbyListQuery> with_lower(const String &p_key, int p_value);
	Ref<HBLobbyListQuery> with_max_results(int p_max_results);
	Ref<HBLobbyListQuery> with_not_equal(const String &p_key, int p_value);
	Ref<HBLobbyListQuery> with_slots_available(int p_min_slots);
	Ref<HBLobbyListQuery> request_lobby_list();
};

class HBSteamMatchmaking : public RefCounted {
	GDCLASS(HBSteamMatchmaking, RefCounted);
	ISteamMatchmaking *steam_matchmaking = nullptr;

protected:
	static void _bind_methods();

public:
	void init_interface();
	bool is_valid() const;
	ISteamMatchmaking *get_interface() const;
	Ref<HBLobbyListQuery> create_lobby_list_query();
};

#endif // STEAM_MATCHMAKING_H
