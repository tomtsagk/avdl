#include "avdl_achievements.h"
#include "avdl_steam.h"

#ifdef AVDL_STEAM
struct avdl_achievements *avdl_achievements_create() {
	if (!SteamUserStats()->RequestCurrentStats()) {
		printf("avdl: failed to initialise achievements\n");
	}
	return 0;
}

void avdl_achievements_clean(struct avdl_achievements *o) {
}

void avdl_achievements_set(struct avdl_achievements *o, const char *achievementId) {
	if (!SteamUserStats()->SetAchievement(achievementId)) {
		printf("avdl: failed to set steam achievement: '%s'\n", achievementId);
	}
	if (!SteamUserStats()->StoreStats()) {
		printf("avdl: failed to upload steam achievement: '%s'\n", achievementId);
	}
}

void avdl_achievements_unset(struct avdl_achievements *o, const char *achievementId) {
	if (!SteamUserStats()->ClearAchievement(achievementId)) {
		printf("avdl: failed to unset steam achievement: '%s'\n", achievementId);
	}
}
#endif
