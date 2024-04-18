/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "core/config/project_settings.h"
#include "steamworks.h"
#include "steamworks_constants.gen.h"

Steamworks *steamworks_singleton;
void initialize_steamworks_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		GLOBAL_DEF("eirteam/steamworks/app_id", -1);
		steamworks_singleton = memnew(Steamworks);
		Engine::get_singleton()->add_singleton(Engine::Singleton("Steamworks", steamworks_singleton));

		if (Engine::get_singleton()->is_editor_hint()) {
			return;
		}

		int app_id = GLOBAL_GET("eirteam/steamworks/app_id");
		if (app_id == -1) {
			return;
		}
		steamworks_singleton->init(app_id);
	}

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_ABSTRACT_CLASS(Steamworks);
	GDREGISTER_ABSTRACT_CLASS(HBSteamInput);
	GDREGISTER_ABSTRACT_CLASS(HBSteamFriends);
	GDREGISTER_ABSTRACT_CLASS(HBSteamFriend);
	GDREGISTER_ABSTRACT_CLASS(HBSteamLobby);
	GDREGISTER_ABSTRACT_CLASS(HBSteamMatchmaking);
	GDREGISTER_ABSTRACT_CLASS(HBLobbyListQuery);
	GDREGISTER_ABSTRACT_CLASS(SteamworksConstants);
	GDREGISTER_ABSTRACT_CLASS(HBSteamNetworking);
	GDREGISTER_ABSTRACT_CLASS(SteamP2PPacket);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCQuery);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCItem);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCAdditionalPreview);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCQueryPageResult);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCEditor);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGC);
	GDREGISTER_ABSTRACT_CLASS(HBSteamRemoteStorage);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUtils);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUser);
	GDREGISTER_ABSTRACT_CLASS(HBAuthTicketForWebAPI);
	GDREGISTER_ABSTRACT_CLASS(HBSteamApps);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUserStats);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCUserItemVoteResult);
	GDREGISTER_ABSTRACT_CLASS(HBSteamUGCItemUpdateProgress);
	GDREGISTER_ABSTRACT_CLASS(HBSteamNetworkingMessages);
	GDREGISTER_ABSTRACT_CLASS(HBSteamNetworkingMessage);
}

void uninitialize_steamworks_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
		return;
	}
	Steamworks *singleton = Steamworks::get_singleton();

	if (singleton != nullptr) {
		memdelete(singleton);
	}
}
