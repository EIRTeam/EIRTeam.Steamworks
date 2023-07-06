/**************************************************************************/
/*  steam_networking.h                                                    */
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

#ifndef STEAM_NETWORKING_H
#define STEAM_NETWORKING_H

#include "core/object/ref_counted.h"
#include "steamworks_callback_data.h"
#include "steamworks_constants.gen.h"

class ISteamNetworking;
class HBSteamFriend;

class SteamP2PPacket : public RefCounted {
	GDCLASS(SteamP2PPacket, RefCounted);
	Vector<uint8_t> data;
	uint64_t sender_steam_id;

protected:
	static void _bind_methods();

public:
	Vector<uint8_t> get_data();
	Ref<HBSteamFriend> get_sender();
	SteamP2PPacket(Vector<uint8_t> p_data, uint64_t p_sender_steam_id);
};

class HBSteamNetworking : public RefCounted {
	GDCLASS(HBSteamNetworking, RefCounted);
	ISteamNetworking *steam_networking = nullptr;
	void _on_p2p_connection_failed(Ref<SteamworksCallbackData> p_callback);
	void _on_p2p_session_request(Ref<SteamworksCallbackData> p_callback);

protected:
	static void _bind_methods();

public:
	bool accept_p2p_session_with_user(Ref<HBSteamFriend> p_user);
	void allow_p2p_packet_relay(bool p_allow_packet_relay);
	bool close_p2p_session_with_user(Ref<HBSteamFriend> p_user);
	bool is_p2p_packet_available(int p_channel = 0);
	Ref<SteamP2PPacket> read_p2p_packet(int p_channel = 0);
	bool send_p2p_packet(Ref<HBSteamFriend> p_target_user, Vector<uint8_t> p_data, SWC::P2PSend p_send_type = SWC::P2P_SEND_RELIABLE, int p_channel = 0);
	void init_interface();
	bool is_valid() const;
};

#endif // STEAM_NETWORKING_H
