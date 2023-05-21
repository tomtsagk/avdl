#include "avdl_achievements.h"
#include "avdl_steam.h"
#include "dd_log.h"

#if defined(AVDL_QUEST2)
#include <OVR_Platform.h>
#endif

#if defined(AVDL_STEAM) || defined(AVDL_QUEST2)
struct avdl_achievements *avdl_achievements_create() {

	#if defined(AVDL_STEAM)
	if (!SteamUserStats()->RequestCurrentStats()) {
		printf("avdl: failed to initialise achievements\n");
	}
	#endif

	return 0;
}

void avdl_achievements_clean(struct avdl_achievements *o) {
}

void avdl_achievements_set(struct avdl_achievements *o, const char *achievementId) {

	#if defined(AVDL_STEAM)
	if (!SteamUserStats()->SetAchievement(achievementId)) {
		printf("avdl: failed to set steam achievement: '%s'\n", achievementId);
	}
	if (!SteamUserStats()->StoreStats()) {
		printf("avdl: failed to upload steam achievement: '%s'\n", achievementId);
	}
	#elif defined(AVDL_QUEST2)
	ovr_Achievements_Unlock(achievementId);
	#endif
}

void avdl_achievements_unset(struct avdl_achievements *o, const char *achievementId) {
	#if defined(AVDL_STEAM)
	if (!SteamUserStats()->ClearAchievement(achievementId)) {
		printf("avdl: failed to unset steam achievement: '%s'\n", achievementId);
	}
	#endif
}
#endif
