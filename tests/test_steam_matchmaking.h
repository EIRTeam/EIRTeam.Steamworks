/**************************************************************************/
/*  test_steam_matchmaking.h                                              */
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

#ifndef TEST_STEAM_MATCHMAKING_H
#define TEST_STEAM_MATCHMAKING_H

#include "../steamworks.h"
#include "test_steamworks.h"
#include "tests/test_macros.h"

namespace TestSteamMatchmaking {
class MatchmakingSignalTester : public RefCounted {
public:
	bool got_lobby_creation_signal = false;
	void _on_test_lobby_creation(SWC::Result p_result) {
		got_lobby_creation_signal = true;
	}

	bool got_lobby_entered_signal = false;
	void _on_test_lobby_entered(bool p_success) {
		got_lobby_entered_signal = true;
	}

	bool got_lobby_list_signal = false;
	void _on_test_lobby_list(TypedArray<HBSteamLobby> p_lobby_list) {
		got_lobby_list_signal = true;
	}

	void connect_signals_to_lobby(Ref<HBSteamLobby> p_lobby) {
		p_lobby->connect("lobby_created", callable_mp(this, &MatchmakingSignalTester::_on_test_lobby_creation));
		p_lobby->connect("lobby_entered", callable_mp(this, &MatchmakingSignalTester::_on_test_lobby_entered));
	}
};

TEST_CASE("[SteamMatchmaking] Test lobby creation/entered signal") {
	TestSteamworks::reinit_steamworks_if_needed();
	Steamworks *singleton = Steamworks::get_singleton();
	singleton->set_run_callbacks_automatically(false);

	Ref<HBSteamMatchmaking> matchmaking = singleton->get_matchmaking();
	REQUIRE(matchmaking->is_valid());

	Ref<MatchmakingSignalTester> signal_tester;
	signal_tester.instantiate();

	Ref<HBSteamLobby> lobby = HBSteamLobby::create_lobby(SWC::LOBBY_TYPE_PRIVATE, 5);
	signal_tester->connect_signals_to_lobby(lobby);
	// Allow four tries half a second apart
	for (int i = 0; i < 4; i++) {
		singleton->run_callbacks();
		if (signal_tester->got_lobby_creation_signal) {
			break;
		}
		OS::get_singleton()->delay_usec(500000);
	}
	CHECK_MESSAGE(signal_tester->got_lobby_creation_signal, "Lobby should trigger the lobby created signal.");
	CHECK_MESSAGE(signal_tester->got_lobby_entered_signal, "Lobby should trigger the lobby entered signal.");
}

TEST_CASE("[SteamMatchmaking] Test lobby listing") {
	TestSteamworks::reinit_steamworks_if_needed();
	Steamworks *singleton = Steamworks::get_singleton();
	singleton->set_run_callbacks_automatically(false);

	Ref<HBSteamMatchmaking> matchmaking = singleton->get_matchmaking();
	REQUIRE(matchmaking->is_valid());

	Ref<MatchmakingSignalTester> signal_tester;
	signal_tester.instantiate();

	Ref<HBLobbyListQuery> query = matchmaking->create_lobby_list_query()
										  ->filter_distance_close()
										  ->filter_distance_far()
										  ->filter_distance_worldwide()
										  ->order_by_near("test", 0)
										  ->with_equal("test", 0)
										  ->with_max_results(5)
										  ->with_slots_available(5)
										  ->request_lobby_list();
	CHECK_MESSAGE(query.is_valid(), "List query must be valid");
	query->connect("received_lobby_list", callable_mp(signal_tester.ptr(), &MatchmakingSignalTester::_on_test_lobby_list));

	for (int i = 0; i < 4; i++) {
		singleton->run_callbacks();
		if (signal_tester->got_lobby_list_signal) {
			break;
		}
		OS::get_singleton()->delay_usec(500000);
	}
	CHECK_MESSAGE(signal_tester->got_lobby_list_signal, "Lobby query should trigger lobby list received signal.");
}
} //namespace TestSteamMatchmaking

#endif // TEST_STEAM_MATCHMAKING_H
