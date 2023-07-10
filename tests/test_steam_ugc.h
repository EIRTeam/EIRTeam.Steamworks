/**************************************************************************/
/*  test_steam_ugc.h                                                      */
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

#ifndef TEST_STEAM_UGC_H
#define TEST_STEAM_UGC_H

#include "test_steamworks.h"
#include "tests/test_macros.h"
#include "tests/test_tools.h"

namespace TestSteamUGC {
const int64_t UGC_ITEM_ID = 2189982812;
class SteamUGCSignalTester : public RefCounted {
public:
	bool got_query_completed_signal = false;
	Ref<HBSteamUGCQueryPageResult> query_completed_result;
	void _on_query_completed(int p_page, Ref<HBSteamUGCQueryPageResult> p_result) {
		got_query_completed_signal = true;
		query_completed_result = p_result;
	}
};

TEST_CASE("[SteamUGC] Test UGC querying") {
	TestSteamworks::reinit_steamworks_if_needed();

	Ref<SteamUGCSignalTester> signal_tester;
	signal_tester.instantiate();

	Ref<HBSteamUGC> steam_ugc = Steamworks::get_singleton()->get_ugc();
	CHECK_MESSAGE(steam_ugc->is_valid(), "Steam UGC interface should be valid");

	Ref<HBSteamUGCQuery> request = HBSteamUGCQuery::create_query(SWC::UGC_MATCHING_UGC_TYPE_ITEMS_READY_TO_USE);
	CHECK_MESSAGE(request.is_valid(), "Request should be valid");

	Vector<int64_t> file_ids;
	file_ids.push_back(UGC_ITEM_ID);
	request = request->with_file_ids(file_ids);
	request->request_page(1);
	request->connect("query_completed", callable_mp(signal_tester.ptr(), &SteamUGCSignalTester::_on_query_completed));

	for (int i = 0; i < 40; i++) {
		Steamworks::get_singleton()->run_callbacks();
		if (signal_tester->got_query_completed_signal) {
			break;
		}
		OS::get_singleton()->delay_usec(50000);
	}
	CHECK_MESSAGE(signal_tester->got_query_completed_signal, "Request should emit the query completed signal.");
	CHECK_MESSAGE(signal_tester->query_completed_result.is_valid(), "Request should return a valid result.");
	CHECK_MESSAGE(signal_tester->query_completed_result->get_results().size() > 0, "Request should return a non-empty result list.");
	Ref<HBSteamUGCItem> item = signal_tester->query_completed_result->get_results()[0];
	CHECK_MESSAGE(item.is_valid(), "Resulting item should be valid.");
	CHECK_MESSAGE(!item->get_title().is_empty(), "UGC item title should not be empty.");
}
TEST_CASE("[SteamUGC] Test UGC ALL query") {
	TestSteamworks::reinit_steamworks_if_needed();
	ErrorDetector ed;
	Ref<HBSteamUGCQuery> request = HBSteamUGCQuery::create_query(SWC::UGC_MATCHING_UGC_TYPE_ITEMS_READY_TO_USE);
	request->request_page(1);
	CHECK_FALSE(ed.has_error);
}
TEST_CASE("[SteamUGC] Test User UGC query") {
	TestSteamworks::reinit_steamworks_if_needed();
	ErrorDetector ed;
	Ref<HBSteamUGCQuery> request = HBSteamUGCQuery::create_query(SWC::UGC_MATCHING_UGC_TYPE_ITEMS_READY_TO_USE);
	request->with_user(Steamworks::get_singleton()->get_user()->get_local_user());
	request->request_page(1);
	CHECK_FALSE(ed.has_error);
}
} //namespace TestSteamUGC

#endif // TEST_STEAM_UGC_H
