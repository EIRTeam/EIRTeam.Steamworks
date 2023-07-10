/**************************************************************************/
/*  steam_user.h                                                          */
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

#ifndef STEAM_USER_H
#define STEAM_USER_H

#include "core/object/ref_counted.h"
#include "steam_friends.h"
#include "steamworks_callback_data.h"
#include "steamworks_constants.gen.h"

class ISteamUser;
class HBSteamUser;

class HBAuthTicketForWebAPI : public RefCounted {
	GDCLASS(HBAuthTicketForWebAPI, RefCounted);
	Vector<uint8_t> ticket_data;
	SWC::HAuthTicket auth_ticket_handle;
	void _on_get_ticket(Ref<SteamworksCallbackData> p_callback);

protected:
	static void _bind_methods();

public:
	Vector<uint8_t> get_ticket_data() const;
	HBAuthTicketForWebAPI(SWC::HAuthTicket p_ticket);
	friend class HBSteamUser;
};

class HBSteamUser : public RefCounted {
	GDCLASS(HBSteamUser, RefCounted);
	ISteamUser *steam_user = nullptr;

protected:
	static void _bind_methods();

public:
	Ref<HBAuthTicketForWebAPI> get_auth_ticket_for_web_api(const String &p_identity) const;
	void init_interface();
	bool is_valid() const;
	Ref<HBSteamFriend> get_local_user() const;
};

#endif // STEAM_USER_H
