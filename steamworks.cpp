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
#include "steamworks_constants.gen.h"
#include "sw_error_macros.h"
#include <iostream>

Steamworks *Steamworks::singleton = nullptr;
String Steamworks::last_error = "";

extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText) {
	if (nSeverity >= 1) {
		ERR_PRINT_ED(pchDebugText);
	} else {
		WARN_PRINT_ED(pchDebugText);
	}
}

void Steamworks::_run_callbacks() {
	SteamAPI_ManualDispatch_RunFrame(steam_pipe);
	CallbackMsg_t msg;
	while (SteamAPI_ManualDispatch_GetNextCallback(steam_pipe, &msg)) {
		if (msg.m_iCallback == SteamAPICallCompleted_t::k_iCallback) {
			SteamAPICallCompleted_t *api_call = (SteamAPICallCompleted_t *)msg.m_pubParam;
			Ref<SteamworksCallbackData> callback_data = memnew(SteamworksCallbackData(msg));
			bool failed;
			bool api_call_ok = SteamAPI_ManualDispatch_GetAPICallResult(steam_pipe, api_call->m_hAsyncCall, callback_data->get_ptr(), msg.m_cubParam, msg.m_iCallback, &failed);
			if (!api_call_ok) {
				SteamAPI_ManualDispatch_FreeLastCallback(steam_pipe);
				ERR_PRINT("API call failed");
				continue;
			}

			if (call_result_callbacks.has(api_call->m_hAsyncCall)) {
				SteamworksCallbackInfo &info = call_result_callbacks[api_call->m_hAsyncCall];
				for (Callable callable : info.callbacks) {
					if (!callable.is_valid()) {
						// API result callbacks are one-time only.
						continue;
					}
					Array args;
					args.push_back(callback_data);
					args.push_back(failed);
					callable.callv(args);
				}
				info.callbacks.clear();
			}
		} else {
			if (callback_infos.has(msg.m_iCallback)) {
				Ref<SteamworksCallbackData> callback_data = memnew(SteamworksCallbackData(msg));
				for (Callable callable : callback_infos[msg.m_iCallback].callbacks) {
					if (!callable.is_valid()) {
						continue;
					}

					Array args;
					args.push_back(callback_data);
					callable.callv(args);
				}
			}
		}
		SteamAPI_ManualDispatch_FreeLastCallback(steam_pipe);
	}
}

void Steamworks::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "app_id", "run_callbacks_automatically"), &Steamworks::init, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_valid"), &Steamworks::is_valid);

	ClassDB::bind_method(D_METHOD("run_callbacks"), &Steamworks::run_callbacks);
	ClassDB::bind_method(D_METHOD("get_input"), &Steamworks::get_input);
	ClassDB::bind_method(D_METHOD("get_local_user"), &Steamworks::get_local_user);

	ClassDB::bind_static_method("Steamworks", D_METHOD("get_last_error"), &Steamworks::get_last_error);
}

void Steamworks::add_callback(int p_callback_type, Callable p_callable) {
	if (!callback_infos.has(p_callback_type)) {
		callback_infos.insert(p_callback_type, SteamworksCallbackInfo());
	}
	SteamworksCallbackInfo &info = callback_infos[p_callback_type];
	info.callbacks.push_back(p_callable);
}

void Steamworks::add_call_result_callback(ResultCallbackType p_callback_id, Callable p_callable) {
	SteamworksCallbackInfo callback_info;
	callback_info.callbacks.push_back(p_callable);
	call_result_callbacks.insert(p_callback_id, callback_info);
}

bool Steamworks::init(int p_app_id, bool p_run_callbacks_automatically) {
	SW_ERR_FAIL_COND_V_MSG(initialized, false, "Steamworks: Calling Steamworks.init but it's already initialized.");

	OS::get_singleton()->set_environment("SteamAppId", Variant(p_app_id));
	OS::get_singleton()->set_environment("SteamGameId", Variant(p_app_id));
	SW_ERR_FAIL_COND_V_MSG(!SteamAPI_Init(), false, "Steamworks: SteamApi_Init returned false. Steam isn't running, couldn't find Steam, App ID is ureleased, Don't own App ID.");
	SteamAPI_ManualDispatch_Init();
	steam_pipe = SteamAPI_GetHSteamPipe();
	initialized = true;
	app_id = p_app_id;

	// Setup debug Steamworks API hooks
	SteamAPI_ISteamClient_SetWarningMessageHook(SteamClient(), SteamAPIDebugTextHook);

	// Initialize steam input singleton
	input = memnew(HBSteamInput);
	input->init_interface();

	matchmaking.instantiate();
	matchmaking->init_interface();

	friends.instantiate();
	friends->init_interface();

	utils.instantiate();
	utils->init_interface();

	set_run_callbacks_automatically(p_run_callbacks_automatically);

	return true;
}

void Steamworks::run_callbacks() {
	_run_callbacks();
	ERR_FAIL_COND_MSG(run_callbacks_automatically, "Steamworks: Called run_callbacks when running callbacks automatically is enabled.");
}

Steamworks::Steamworks() {
	singleton = this;
}

Steamworks::~Steamworks() {
	if (initialized) {
		initialized = false;
		if (input) {
			memdelete(input);
		}
		matchmaking = Ref<HBSteamMatchmaking>();
		friends = Ref<HBSteamFriends>();
		utils = Ref<HBSteamUtils>();
		SteamAPI_Shutdown();
	}
	singleton = nullptr;
}

bool Steamworks::get_run_callbacks_automatically() const {
	return run_callbacks_automatically;
}

void Steamworks::set_run_callbacks_automatically(bool p_run_callbacks_automatically) {
	if (p_run_callbacks_automatically == run_callbacks_automatically) {
		return;
	}

	Callable callable = callable_mp(this, &Steamworks::_run_callbacks);

	if (p_run_callbacks_automatically && SceneTree::get_singleton()) {
		WARN_PRINT_ONCE("Steamworks: set_run_callbacks_automatically called with true before the SceneTree was initialized, this is not supported.");
		return;
	}

	if (SceneTree::get_singleton()->is_connected("process_frame", callable)) {
		SceneTree::get_singleton()->disconnect("process_frame", callable);
	} else {
		SceneTree::get_singleton()->connect("process_frame", callable);
	}

	run_callbacks_automatically = p_run_callbacks_automatically;
}

HBSteamInput *Steamworks::get_input() const {
	return input;
}

Ref<HBSteamMatchmaking> Steamworks::get_matchmaking() const {
	return matchmaking;
}

Ref<HBSteamFriends> Steamworks::get_friends() const {
	return friends;
}

Ref<HBSteamUtils> Steamworks::get_utils() const {
	return utils;
}

Ref<HBSteamFriend> Steamworks::get_local_user() const {
	return HBSteamFriend::from_steam_id(SteamAPI_ISteamUser_GetSteamID(SteamAPI_SteamUser()));
}
