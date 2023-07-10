/**************************************************************************/
/*  steam_apps.cpp                                                        */
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

#include "steam_apps.h"
#include "steam/steam_api_flat.h"

bool HBSteamApps::is_subscribed() const {
	return SteamAPI_ISteamApps_BIsSubscribed(steam_apps);
}

bool HBSteamApps::is_subscribed_app(uint64_t p_app_id) const {
	return SteamAPI_ISteamApps_BIsSubscribedApp(steam_apps, p_app_id);
}

bool HBSteamApps::is_app_installed(uint64_t p_app_id) const {
	return SteamAPI_ISteamApps_BIsAppInstalled(steam_apps, p_app_id);
}

String HBSteamApps::get_app_install_dir(uint64_t p_app_id) const {
	char bytes[256];
	uint32_t dir_length = SteamAPI_ISteamApps_GetAppInstallDir(steam_apps, p_app_id, bytes, 256);
	return String::utf8(bytes, dir_length);
}

void HBSteamApps::init_interface() {
	steam_apps = SteamAPI_SteamApps();
}

bool HBSteamApps::is_valid() const {
	return steam_apps != nullptr;
}
