/**************************************************************************/
/*  test_steamworks.h                                                     */
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

#ifndef TESTS_STEAMWORKS_H
#define TESTS_STEAMWORKS_H

#include "../steamworks.h"
#include "tests/test_macros.h"

namespace TestSteamworks {
static const int TEST_APPID = 1216230;
static void reinit_steamworks_if_needed() {
	Steamworks *singleton = Steamworks::get_singleton();
	CHECK(singleton);
	if (!singleton->is_valid()) {
		CHECK(singleton->init(TEST_APPID, false));
		CHECK(singleton->is_valid());
	}
}
TEST_CASE("[Steamworks] Test initialization failure") {
	// Hack-ish, deinit steamworks first before testing
	Steamworks *singleton = Steamworks::get_singleton();
	if (singleton != nullptr) {
		memdelete(Steamworks::get_singleton());
	}
	singleton = memnew(Steamworks);

	CHECK_MESSAGE(singleton != nullptr, "Steamworks singleton should not be null");
	ERR_PRINT_OFF;
	CHECK_FALSE_MESSAGE(singleton->init(-1, false), "Steamworks should NOT initialize correctly when given an incorrect App ID.");
	CHECK_FALSE_MESSAGE(singleton->is_valid(), "is_valid() should return false when Steamworks has been initialized incorrectly.");
	CHECK_FALSE_MESSAGE(singleton->get_last_error().is_empty(), "Steamworks get_last_error should not be empty after an initialization failure.");
	ERR_PRINT_ON;
}
TEST_CASE("[Steamworks] Test correct initialization") {
	// Hack-ish, deinit steamworks first before testing
	Steamworks *singleton = Steamworks::get_singleton();
	if (singleton != nullptr) {
		memdelete(Steamworks::get_singleton());
	}
	singleton = memnew(Steamworks);

	CHECK_MESSAGE(singleton != nullptr, "Steamworks singleton should not be null.");
	CHECK_MESSAGE(singleton->init(TEST_APPID, false), "Steamworks should initialize correctly when given a correct App ID.");
	CHECK_MESSAGE(singleton->is_valid(), "is_valid() should return true when Steamworks has been initialized correctly.");
	CHECK_MESSAGE(singleton->get_input() != nullptr, "SteamInput node should be valid.");
	CHECK_MESSAGE(singleton->get_matchmaking().is_valid(), "SteamMatchmaking interface should be valid.");
	CHECK_MESSAGE(singleton->get_friends().is_valid(), "SteamFriends interface should be valid");
	CHECK_MESSAGE(singleton->get_utils().is_valid(), "SteamUtils interface should be valid");
	CHECK_MESSAGE(singleton->get_networking().is_valid(), "SteamNetworking interface should be valid");
	CHECK_MESSAGE(singleton->get_ugc().is_valid(), "SteamUGC interface should be valid");
	CHECK_MESSAGE(singleton->get_remote_storage().is_valid(), "SteamRemoteStorage interface should be valid");
	CHECK_MESSAGE(singleton->get_user().is_valid(), "SteamUser interface should be valid");
	CHECK_MESSAGE(singleton->get_user_stats().is_valid(), "SteamUserStats interface should be valid");
}
TEST_CASE("[Steamworks] Test getting the local user") {
	reinit_steamworks_if_needed();
	Steamworks *singleton = Steamworks::get_singleton();

	Ref<HBSteamFriend> user = singleton->get_user()->get_local_user();
	CHECK_MESSAGE(user.is_valid(), "Local user should be valid.");
}
TEST_CASE("[Steamworks] Check basic steamworks interactions") {
	reinit_steamworks_if_needed();
	Steamworks *singleton = Steamworks::get_singleton();

	SUBCASE("[Steawmorks] Test getting local user") {
		Ref<HBSteamFriend> local_user = singleton->get_user()->get_local_user();
		CHECK_MESSAGE(local_user.is_valid(), "Local user must be valid");
	}
}
} //namespace TestSteamworks

#endif // TESTS_STEAMWORKS_H
