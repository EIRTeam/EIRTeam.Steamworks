/**************************************************************************/
/*  test_steam_networking.h                                               */
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

#ifndef TEST_STEAM_NETWORKING_H
#define TEST_STEAM_NETWORKING_H

#include "test_steamworks.h"
#include "tests/test_macros.h"

namespace TestSteamNetworking {
TEST_CASE("[SteamNetworking] Test sending an receiving Steam P2P packets") {
	TestSteamworks::reinit_steamworks_if_needed();
	Ref<HBSteamFriend> local_user = Steamworks::get_singleton()->get_user()->get_local_user();
	Ref<HBSteamNetworking> networking = Steamworks::get_singleton()->get_networking();
	Vector<uint8_t> test_data;
	test_data.push_back(1);
	test_data.push_back(2);
	test_data.push_back(3);
	bool sent = networking->send_p2p_packet(local_user, test_data);
	CHECK_MESSAGE(sent, "A P2P packet should have been sent.");
	for (int i = 0; i < 40; i++) {
		Steamworks::get_singleton()->run_callbacks();
		if (networking->is_p2p_packet_available()) {
			break;
		}
		OS::get_singleton()->delay_usec(50000);
	}
	CHECK_MESSAGE(networking->is_p2p_packet_available(), "A P2P packet should be available.");
	Ref<SteamP2PPacket> packet = networking->read_p2p_packet();
	CHECK_MESSAGE(packet.is_valid(), "The received P2P packet should be valid");
	CHECK_MESSAGE(packet->get_sender() == local_user, "The received P2P packet's sender should be the local user.");
	CHECK_MESSAGE(packet->get_data().size() == 3, "The received P2P packet should be the same length as the sent one.");
	CHECK_MESSAGE(packet->get_data()[1] == 2, "The received P2P packet should match the sent data.");
}
} //namespace TestSteamNetworking

#endif // TEST_STEAM_NETWORKING_H
