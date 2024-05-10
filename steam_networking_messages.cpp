#include "steam_networking_messages.h"
#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

void HBSteamNetworkingMessages::_on_session_requested(Ref<SteamworksCallbackData> p_callback_data) {
	SteamNetworkingMessagesSessionRequest_t *request = (SteamNetworkingMessagesSessionRequest_t *)p_callback_data->get_data<SteamNetworkingMessagesSessionRequest_t>();
	uint64_t steam_id = SteamAPI_SteamNetworkingIdentity_GetSteamID64(&request->m_identityRemote);
	emit_signal("session_requested", HBSteamFriend::from_steam_id(steam_id));
}

void HBSteamNetworkingMessages::_on_session_failed(Ref<SteamworksCallbackData> p_callback_data) {
	SteamNetworkingMessagesSessionFailed_t *request = (SteamNetworkingMessagesSessionFailed_t *)p_callback_data->get_data<SteamNetworkingMessagesSessionFailed_t>();
	uint64_t steam_id = SteamAPI_SteamNetworkingIdentity_GetSteamID64(&request->m_info.m_identityRemote);
	emit_signal("session_failed", request->m_info.m_eEndReason, HBSteamFriend::from_steam_id(steam_id));
}

void HBSteamNetworkingMessages::_bind_methods() {
	ClassDB::bind_method(D_METHOD("poll_messages", "local_channel"), &HBSteamNetworkingMessages::poll_messages);
	ClassDB::bind_method(D_METHOD("send_message_to_user", "data", "target_user", "send_flags", "channel"), &HBSteamNetworkingMessages::send_message_to_user);
	ClassDB::bind_method(D_METHOD("accept_session_with_user", "user"), &HBSteamNetworkingMessages::accept_session_with_user);
	ADD_SIGNAL(MethodInfo("session_requested", PropertyInfo(Variant::OBJECT, "sender", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
	ADD_SIGNAL(MethodInfo("session_failed", PropertyInfo(Variant::INT, "end_reason"), PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend")));
}

void HBSteamNetworkingMessages::init_interface() {
	steam_networking_messages = SteamAPI_SteamNetworkingMessages_SteamAPI();
	SW_ERR_FAIL_COND_MSG(steam_networking_messages == nullptr, "Steamworks: Failed to initialize Steam networking messages, something catastrophic must have happened");
	Steamworks::get_singleton()->add_callback(SteamNetworkingMessagesSessionRequest_t::k_iCallback, callable_mp(this, &HBSteamNetworkingMessages::_on_session_requested));
	Steamworks::get_singleton()->add_callback(SteamNetworkingMessagesSessionFailed_t::k_iCallback, callable_mp(this, &HBSteamNetworkingMessages::_on_session_failed));
}

bool HBSteamNetworkingMessages::is_valid() const {
	return steam_networking_messages != nullptr;
}

SWC::Result HBSteamNetworkingMessages::send_message_to_user(PackedByteArray p_data, Ref<HBSteamFriend> p_target_user, int p_send_flags, int p_channel) {
	ISteamNetworkingMessages *nm = get_interface();
	ERR_FAIL_COND_V(!p_target_user.is_valid(), SWC::RESULT_FAIL);
	SteamNetworkingIdentity identity;
	SteamAPI_SteamNetworkingIdentity_Clear(&identity);
	SteamAPI_SteamNetworkingIdentity_SetSteamID64(&identity, p_target_user->get_steam_id());
	return (SWC::Result)SteamAPI_ISteamNetworkingMessages_SendMessageToUser(nm, identity, p_data.ptr(), p_data.size(), p_send_flags, p_channel);
}

bool HBSteamNetworkingMessages::accept_session_with_user(Ref<HBSteamFriend> p_user) {
	ISteamNetworkingMessages *nm = get_interface();
	ERR_FAIL_COND_V(!p_user.is_valid(), false);
	SteamNetworkingIdentity identity;
	SteamAPI_SteamNetworkingIdentity_Clear(&identity);
	SteamAPI_SteamNetworkingIdentity_SetSteamID64(&identity, p_user->get_steam_id());

	return SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser(nm, identity);
}

TypedArray<HBSteamNetworkingMessage> HBSteamNetworkingMessages::poll_messages(int p_local_channel) {
	ISteamNetworkingMessages *nm = get_interface();

	constexpr int MAX_MESSAGES = 32;
	Vector<SteamNetworkingMessage_t *> messages;
	messages.resize(32);

	int message_count = SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel(nm, p_local_channel, messages.ptrw(), MAX_MESSAGES);

	TypedArray<HBSteamNetworkingMessage> out;
	out.resize(message_count);
	for (int i = 0; i < message_count; i++) {
		Ref<HBSteamNetworkingMessage> message = HBSteamNetworkingMessage::create_from_message(messages[i]);
		SteamAPI_SteamNetworkingMessage_t_Release(messages[i]);
		out[i] = message;
	}
	return out;
}

ISteamNetworkingMessages *HBSteamNetworkingMessages::get_interface() const {
	return steam_networking_messages;
}

void HBSteamNetworkingMessage::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sender"), &HBSteamNetworkingMessage::get_sender);
	ClassDB::bind_method(D_METHOD("get_data"), &HBSteamNetworkingMessage::get_data);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sender", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), "", "get_sender");
}

Ref<HBSteamNetworkingMessage> HBSteamNetworkingMessage::create_from_message(SteamNetworkingMessage_t *p_message) {
	Ref<HBSteamNetworkingMessage> message;
	message.instantiate();
	message->data.resize(p_message->m_cbSize);
	memcpy(message->data.ptrw(), p_message->m_pData, p_message->m_cbSize);
	uint64_t steam_id = SteamAPI_SteamNetworkingIdentity_GetSteamID64(&p_message->m_identityPeer);
	message->sender = HBSteamFriend::from_steam_id(steam_id);
	return message;
}
