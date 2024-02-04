#include "steam_user_stats.h"
#include "steam/steam_api_flat.h"

void HBSteamUserStats::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_achievement", "achievement"), &HBSteamUserStats::set_achievement);
	ClassDB::bind_method(D_METHOD("store_stats"), &HBSteamUserStats::store_stats);
}

bool HBSteamUserStats::set_achievement(const String &p_achivement) const {
	return SteamAPI_ISteamUserStats_SetAchievement(steam_user_stats, p_achivement.utf8());
}

bool HBSteamUserStats::store_stats() const {
	return SteamAPI_ISteamUserStats_StoreStats(steam_user_stats);
}

void HBSteamUserStats::init_interface() {
	steam_user_stats = SteamAPI_SteamUserStats();
}

bool HBSteamUserStats::is_valid() const {
	return steam_user_stats != nullptr;
}
