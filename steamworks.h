/**************************************************************************/
/*  steamworks.h                                                          */
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

#ifndef STEAMWORKS_H
#define STEAMWORKS_H

#include "steam_apps.h"
#include "steam_friends.h"
#include "steam_input.h"
#include "steam_matchmaking.h"
#include "steam_networking.h"
#include "steam_networking_messages.h"
#include "steam_remote_storage.h"
#include "steam_ugc.h"
#include "steam_user.h"
#include "steam_user_stats.h"
#include "steam_utils.h"

class ISteamClient;
class Steamworks : public Object {
	GDCLASS(Steamworks, Object);

	SWC::HSteamPipe steam_pipe;
	ISteamClient *steam_client;
	static Steamworks *singleton;
	bool initialized = false;
	bool run_callbacks_automatically = false;
	int app_id;

	HBSteamInput *input = nullptr;
	Ref<HBSteamMatchmaking> matchmaking;
	Ref<HBSteamFriends> friends;
	Ref<HBSteamUtils> utils;
	Ref<HBSteamNetworking> networking;
	Ref<HBSteamUGC> ugc;
	Ref<HBSteamApps> apps;
	Ref<HBSteamUser> user;
	Ref<HBSteamRemoteStorage> remote_storage;
	Ref<HBSteamUserStats> user_stats;
	Ref<HBSteamNetworkingMessages> networking_messages;
	typedef int CallbackType;

	struct SteamworksCallbackInfo {
		Vector<Callable> callbacks;
	};

	typedef uint64_t ResultCallbackType;

	HashMap<CallbackType, SteamworksCallbackInfo> callback_infos;
	HashMap<ResultCallbackType, SteamworksCallbackInfo> call_result_callbacks;
	void _run_callbacks();
	bool get_ticket_for_web_api(const String &p_identifier) const;

protected:
	static void _bind_methods();

public:
	void add_callback(int p_callback_type, Callable p_callable);
	void add_call_result_callback(uint64_t p_callback_id, Callable p_callable);
	static String last_error;
	static String get_last_error() { return last_error; };
	static Steamworks *get_singleton() { return singleton; }

	bool init(int p_app_id, bool p_run_callbacks_automatically = true);
	bool is_valid() const { return initialized; };

	void run_callbacks();

	bool get_run_callbacks_automatically() const;
	void set_run_callbacks_automatically(bool p_run_callbacks_automatically);

	HBSteamInput *get_input() const;
	Ref<HBSteamMatchmaking> get_matchmaking() const;
	Ref<HBSteamFriends> get_friends() const;
	Ref<HBSteamUtils> get_utils() const;
	Ref<HBSteamNetworking> get_networking() const;
	Ref<HBSteamUGC> get_ugc() const;
	Ref<HBSteamApps> get_apps() const;
	Ref<HBSteamUser> get_user() const;
	Ref<HBSteamRemoteStorage> get_remote_storage() const;
	Ref<HBSteamUserStats> get_user_stats() const;
	Ref<HBSteamNetworkingMessages> get_networking_messages() const;
	int get_app_id() const;

	Steamworks();
	~Steamworks();
};

#endif // STEAMWORKS_H
