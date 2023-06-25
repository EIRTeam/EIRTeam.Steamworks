/**************************************************************************/
/*  steamworks.cpp                                                        */
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

#include "steamworks.h"
#include "scene/main/window.h"
#include "steam/steam_api_common.h"
#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

Steamworks *Steamworks::singleton = nullptr;
String Steamworks::last_error = "";

extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText) {
	if (nSeverity >= 1) {
		ERR_PRINT_ED(pchDebugText);
	} else {
		WARN_PRINT_ED(pchDebugText);
	}
}

void Steamworks::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "app_id", "run_callbacks_automatically"), &Steamworks::init, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_valid"), &Steamworks::is_valid);

	ClassDB::bind_method(D_METHOD("run_callbacks"), &Steamworks::run_callbacks);
	ClassDB::bind_method(D_METHOD("get_input"), &Steamworks::get_input);
	ClassDB::bind_method(D_METHOD("get_last_error"), &Steamworks::get_last_error);
}

void Steamworks::_notification(int p_what) {
}

bool Steamworks::init(int p_app_id, bool p_run_callbacks_automatically) {
	SW_ERR_FAIL_COND_V_MSG(initialized, false, "Steamworks: Calling Steamworks.init but it's already initialized.");

	OS::get_singleton()->set_environment("SteamAppId", Variant(p_app_id));
	OS::get_singleton()->set_environment("SteamGameId", Variant(p_app_id));
	SW_ERR_FAIL_COND_V_MSG(!SteamAPI_Init(), false, "Steamworks: SteamApi_Init returned false. Steam isn't running, couldn't find Steam, App ID is ureleased, Don't own App ID.");
	steam_pipe = SteamAPI_GetHSteamPipe();
	initialized = true;
	app_id = p_app_id;

	// Setup debug Steamworks API hooks
	SteamAPI_ISteamClient_SetWarningMessageHook(SteamClient(), SteamAPIDebugTextHook);

	// Initialize steam input singleton
	input = memnew(HBSteamInput);
	input->init_interface();

	SceneTree::get_singleton()->get_root()->call_deferred("add_child", this);
	add_child(input);

	return true;
}

void Steamworks::run_callbacks() {
	SteamAPI_RunCallbacks();
}

Steamworks::Steamworks() {
	singleton = this;
}

Steamworks::~Steamworks() {
	if (initialized) {
		SteamAPI_Shutdown();
	}
}

HBSteamInput *Steamworks::get_input() const {
	return input;
}
