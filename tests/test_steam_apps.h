/**************************************************************************/
/*  test_steam_apps.h                                                     */
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

#ifndef TEST_STEAM_APPS_H
#define TEST_STEAM_APPS_H

#include "test_steamworks.h"
#include "tests/test_macros.h"

namespace TestSteamApps {
TEST_CASE("[SteamApps] Test app subscription checks") {
	TestSteamworks::reinit_steamworks_if_needed();
	Ref<HBSteamApps> apps = Steamworks::get_singleton()->get_apps();
	CHECK_MESSAGE(apps->is_subscribed(), "Should be subscribed to current app");
	CHECK_MESSAGE(apps->is_subscribed_app(480), "Should be subscribed to the spacewar app.");
	CHECK_MESSAGE(apps->get_app_install_dir(480).length() > 0, "App install dir for spacewar should not be empty.");
}
} //namespace TestSteamApps

#endif // TEST_STEAM_APPS_H
