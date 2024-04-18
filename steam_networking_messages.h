#ifndef STEAM_NETWORKING_MESSAGES_H
#define STEAM_NETWORKING_MESSAGES_H

#include "core/object/ref_counted.h"
#include "steamworks_constants.gen.h"

class ISteamNetworkingMessages;
class HBSteamFriend;
class SteamworksCallbackData;
struct SteamNetworkingMessage_t;

class HBSteamNetworkingMessage : public RefCounted {
	GDCLASS(HBSteamNetworkingMessage, RefCounted);
	PackedByteArray data;
	Ref<HBSteamFriend> sender;

protected:
	static void _bind_methods();

public:
	PackedByteArray get_data() const { return data; }
	static Ref<HBSteamNetworkingMessage> create_from_message(SteamNetworkingMessage_t *p_message);

	Ref<HBSteamFriend> get_sender() const { return sender; }
};

class HBSteamNetworkingMessages : public RefCounted {
	GDCLASS(HBSteamNetworkingMessages, RefCounted);
	ISteamNetworkingMessages *steam_networking_messages = nullptr;
	void _on_session_requested(Ref<SteamworksCallbackData> p_callback_data);
	void _on_session_failed(Ref<SteamworksCallbackData> p_callback_data);

protected:
	static void _bind_methods();

public:
	void init_interface();
	bool is_valid() const;
	SWC::Result send_message_to_user(PackedByteArray p_data, Ref<HBSteamFriend> p_target_user, int p_send_flags, int p_channel);
	bool accept_session_with_user(Ref<HBSteamFriend> p_user);
	TypedArray<HBSteamNetworkingMessage> poll_messages(int p_local_channel);
	ISteamNetworkingMessages *get_interface() const;
};

#endif // STEAM_NETWORKING_MESSAGES_H
