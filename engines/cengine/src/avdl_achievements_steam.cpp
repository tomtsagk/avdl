#include "avdl_achievements.h"
#include "steam_api.h"

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
}
