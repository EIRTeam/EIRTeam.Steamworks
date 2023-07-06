/**************************************************************************/
/*  steam_networking.cpp                                                  */
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

#include "steam_networking.h"
#include "steam/steam_api_flat.h"
#include "steam_friends.h"
#include "steamworks.h"

void HBSteamNetworking::_on_p2p_connection_failed(Ref<SteamworksCallbackData> p_callback) {
	const P2PSessionConnectFail_t *failure = p_callback->get_data<P2PSessionConnectFail_t>();
	const uint64_t *steam_id = (uint64_t *)&failure->m_steamIDRemote;
	emit_signal("p2p_connection_failed", HBSteamFriend::from_steam_id(*steam_id), failure->m_eP2PSessionError);
}

void HBSteamNetworking::_on_p2p_session_request(Ref<SteamworksCallbackData> p_callback) {
	const P2PSessionRequest_t *request = p_callback->get_data<P2PSessionRequest_t>();
	const uint64_t *steam_id = (uint64_t *)&request->m_steamIDRemote;
	emit_signal("p2p_session_requested", HBSteamFriend::from_steam_id(*steam_id));
}

void HBSteamNetworking::_bind_methods() {
	ClassDB::bind_method(D_METHOD("accept_p2p_session_with_user", "user"), &HBSteamNetworking::accept_p2p_session_with_user);
	ClassDB::bind_method(D_METHOD("allow_p2p_packet_relay", "allow_packet_relay"), &HBSteamNetworking::allow_p2p_packet_relay);
	ClassDB::bind_method(D_METHOD("close_p2p_session_with_user", "user"), &HBSteamNetworking::close_p2p_session_with_user);
	ClassDB::bind_method(D_METHOD("is_p2p_packet_available", "channel"), &HBSteamNetworking::is_p2p_packet_available, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("read_p2p_packet", "channel"), &HBSteamNetworking::read_p2p_packet, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("send_p2p_packet", "target_user", "data", "send_type", "channel"), &HBSteamNetworking::send_p2p_packet, DEFVAL(SWC::P2P_SEND_RELIABLE), DEFVAL(0));

	ADD_SIGNAL(MethodInfo("p2p_session_requested", PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
	ADD_SIGNAL(MethodInfo("p2p_connection_failed", PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), PropertyInfo(Variant::INT, "connection_error")));
}

bool HBSteamNetworking::accept_p2p_session_with_user(Ref<HBSteamFriend> p_user) {
	return SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser(steam_networking, p_user->get_steam_id());
}

void HBSteamNetworking::allow_p2p_packet_relay(bool p_allow_packet_relay) {
	SteamAPI_ISteamNetworking_AllowP2PPacketRelay(steam_networking, p_allow_packet_relay);
}

bool HBSteamNetworking::close_p2p_session_with_user(Ref<HBSteamFriend> p_user) {
	return SteamAPI_ISteamNetworking_CloseP2PSessionWithUser(steam_networking, p_user->get_steam_id());
}

bool HBSteamNetworking::is_p2p_packet_available(int p_channel) {
	uint32_t _packet_size;
	return SteamAPI_ISteamNetworking_IsP2PPacketAvailable(steam_networking, &_packet_size, p_channel);
}

Ref<SteamP2PPacket> HBSteamNetworking::read_p2p_packet(int p_channel) {
	uint32_t packet_size;
	if (!SteamAPI_ISteamNetworking_IsP2PPacketAvailable(steam_networking, &packet_size, p_channel)) {
		return Ref<SteamP2PPacket>();
	}

	Vector<uint8_t> packet_data;
	packet_data.resize(packet_size);

	// I can't believe this is legal...
	// CSteamID should be a structure that can be reinterpreted as a 64 bit int, which should be a Steam ID.
	// damn valve using classes in the """FLAT""" API.
	uint64_t sender_steam_id;

	bool read_successful = SteamAPI_ISteamNetworking_ReadP2PPacket(steam_networking, packet_data.ptrw(), packet_data.size(), &packet_size, (CSteamID *)&sender_steam_id, p_channel);
	if (!read_successful) {
		return Ref<SteamP2PPacket>();
	}

	return memnew(SteamP2PPacket(packet_data, sender_steam_id));
}

bool HBSteamNetworking::send_p2p_packet(Ref<HBSteamFriend> p_target_user, Vector<uint8_t> p_data, SWC::P2PSend p_send_type, int p_channel) {
	ERR_FAIL_COND_V_MSG(!p_target_user.is_valid(), false, "Given target user for P2P packet was invalid.");
	ERR_FAIL_COND_V_MSG(p_data.size() == 0, false, "Given P2P packet data to send was empty.");
	return SteamAPI_ISteamNetworking_SendP2PPacket(steam_networking, p_target_user->get_steam_id(), p_data.ptr(), p_data.size(), (EP2PSend)p_send_type, p_channel);
}

void HBSteamNetworking::init_interface() {
	steam_networking = SteamAPI_SteamNetworking();
	Steamworks *sw = Steamworks::get_singleton();
	sw->add_callback(P2PSessionConnectFail_t::k_iCallback, callable_mp(this, &HBSteamNetworking::_on_p2p_connection_failed));
	sw->add_callback(P2PSessionRequest_t::k_iCallback, callable_mp(this, &HBSteamNetworking::_on_p2p_session_request));
}

bool HBSteamNetworking::is_valid() const {
	return steam_networking;
}

void SteamP2PPacket::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_data"), &SteamP2PPacket::get_data);
	ClassDB::bind_method(D_METHOD("get_sender"), &SteamP2PPacket::get_sender);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sender", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), "", "get_sender");
}

Vector<uint8_t> SteamP2PPacket::get_data() {
	return data;
}

Ref<HBSteamFriend> SteamP2PPacket::get_sender() {
	return HBSteamFriend::from_steam_id(sender_steam_id);
}

SteamP2PPacket::SteamP2PPacket(Vector<uint8_t> p_data, uint64_t p_sender_steam_id) {
	sender_steam_id = p_sender_steam_id;
	data = p_data;
}
