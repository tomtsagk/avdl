#ifndef AVDL_ACHIEVEMENTS_H
#define AVDL_ACHIEVEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_achievements {
	int x;
};

struct avdl_achievements *avdl_achievements_create();
void avdl_achievements_clean(struct avdl_achievements *o);

void avdl_achievements_set(struct avdl_achievements *o, const char *achievementId);

#ifdef __cplusplus
}
#endif

#endif
