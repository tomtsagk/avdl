#include "avdl_achievements.h"

#ifndef AVDL_STEAM
struct avdl_achievements *avdl_achievements_create() {
	return 0;
}

void avdl_achievements_clean(struct avdl_achievements *o) {
}

void avdl_achievements_set(struct avdl_achievements *o, const char *achievementId) {
}

void avdl_achievements_unset(struct avdl_achievements *o, const char *achievementId) {
}
#endif
