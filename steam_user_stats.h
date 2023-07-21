#ifndef STEAM_USER_STATS_H
#define STEAM_USER_STATS_H

#include "core/object/ref_counted.h"

class ISteamUserStats;
class HBSteamUserStats : public RefCounted {
	GDCLASS(HBSteamUserStats, RefCounted);

	ISteamUserStats *steam_user_stats = nullptr;

protected:
	static void _bind_methods();

public:
	bool set_achievement(const String &p_achivement) const;
	bool store_stats() const;
	void init_interface();
	bool is_valid() const;
};

#endif // STEAM_USER_STATS_H
