#ifndef DD_MUSIC_H
#define DD_MUSIC_H

#if DD_PLATFORM_ANDROID
#else
#include "dd_opengl.h"

#if defined(_WIN32) || defined(WIN32)
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

#endif

extern int dd_hasAudio;

/* music element
 * meant for background music
 */
struct dd_music {
	#if DD_PLATFORM_ANDROID
	char filename[100];
	int index;
	#else
	Mix_Music *music;
	#endif

	void (*load)(struct dd_music *, const char *filename);
	void (*clean)(struct dd_music *);
	void (*play)(struct dd_music *);
	void (*playLoop)(struct dd_music *, int loops);
	void (*stop)(struct dd_music *);
};

void dd_music_create(struct dd_music *);
void dd_music_load(struct dd_music *, const char *filename);
void dd_music_clean(struct dd_music *);
void dd_music_play(struct dd_music *);
void dd_music_playLoop(struct dd_music *, int loops);
void dd_music_stop(struct dd_music *);

void avdl_music_setVolume(int volume);
int avdl_music_getVolume();

#endif
