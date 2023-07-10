/**************************************************************************/
/*  test_steam_user.h                                                     */
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

#ifndef TEST_STEAM_USER_H
#define TEST_STEAM_USER_H

#include "test_steamworks.h"
#include "tests/test_macros.h"

namespace TestSteamUser {
class SteamUserSignalTester : public RefCounted {
	GDCLASS(SteamUserSignalTester, RefCounted);

public:
	bool got_ticket_received_signal = false;
	void _on_ticket_received(bool p_success) {
		got_ticket_received_signal = true;
	}
};

TEST_CASE("[SteamUser] Test app tickets") {
	TestSteamworks::reinit_steamworks_if_needed();
	Ref<SteamUserSignalTester> signal_tester;
	signal_tester.instantiate();
	Ref<HBAuthTicketForWebAPI> ticket = Steamworks::get_singleton()->get_user()->get_auth_ticket_for_web_api("Test app");
	CHECK_MESSAGE(ticket.is_valid(), "Ticket must be valid");
	ticket->connect("ticket_received", callable_mp(signal_tester.ptr(), &SteamUserSignalTester::_on_ticket_received));
	for (int i = 0; i < 40; i++) {
		Steamworks::get_singleton()->run_callbacks();
		if (signal_tester->got_ticket_received_signal) {
			break;
		}
		OS::get_singleton()->delay_usec(50000);
	}
	CHECK_MESSAGE(signal_tester->got_ticket_received_signal, "Ticket received signal must get emitted");
	CHECK_MESSAGE(ticket->get_ticket_data().size() > 0, "Ticket must not be empty");
}
}; //namespace TestSteamUser

#endif // TEST_STEAM_USER_H
