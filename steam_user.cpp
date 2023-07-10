/**************************************************************************/
/*  steam_user.cpp                                                        */
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

#include "steam_user.h"
#include "steam/steam_api_flat.h"
#include "steamworks.h"

void HBAuthTicketForWebAPI::_on_get_ticket(Ref<SteamworksCallbackData> p_callback) {
	const GetTicketForWebApiResponse_t *response = p_callback->get_data<GetTicketForWebApiResponse_t>();
	if (response->m_eResult == k_EResultOK) {
		ticket_data.resize(response->m_cubTicket);
		memcpy(ticket_data.ptrw(), response->m_rgubTicket, response->m_cubTicket);
	}
	emit_signal("ticket_received", response->m_eResult == k_EResultOK);
}

void HBAuthTicketForWebAPI::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ticket_data"), &HBAuthTicketForWebAPI::get_ticket_data);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "ticket_data"), "", "get_ticket_data");
	ADD_SIGNAL(MethodInfo("ticket_received", PropertyInfo(Variant::BOOL, "success")));
}

Vector<uint8_t> HBAuthTicketForWebAPI::get_ticket_data() const {
	return ticket_data;
}

HBAuthTicketForWebAPI::HBAuthTicketForWebAPI(SWC::HAuthTicket p_ticket) {
	auth_ticket_handle = p_ticket;
	Steamworks::get_singleton()->add_callback(GetTicketForWebApiResponse_t::k_iCallback, callable_mp(this, &HBAuthTicketForWebAPI::_on_get_ticket));
}

void HBSteamUser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_auth_ticket_for_web_api", "identity"), &HBSteamUser::get_auth_ticket_for_web_api);
	ClassDB::bind_method(D_METHOD("get_local_user"), &HBSteamUser::get_local_user);
}

Ref<HBAuthTicketForWebAPI> HBSteamUser::get_auth_ticket_for_web_api(const String &p_identity) const {
	HAuthTicket ticket = SteamAPI_ISteamUser_GetAuthTicketForWebApi(steam_user, p_identity.utf8());
	if (ticket == k_HAuthTicketInvalid) {
		return Ref<HBAuthTicketForWebAPI>();
	}
	return memnew(HBAuthTicketForWebAPI(ticket));
}

void HBSteamUser::init_interface() {
	steam_user = SteamAPI_SteamUser();
}

bool HBSteamUser::is_valid() const {
	return steam_user != nullptr;
}

Ref<HBSteamFriend> HBSteamUser::get_local_user() const {
	return HBSteamFriend::from_steam_id(SteamAPI_ISteamUser_GetSteamID(steam_user));
}