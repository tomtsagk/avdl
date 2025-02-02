#ifndef DD_SOUND_H
#define DD_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#else
#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11

#if defined(_WIN32) || defined(WIN32)
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

#endif

#endif

extern int dd_hasAudio;
extern int dd_numberOfAudioChannels;

extern int avdl_sound_volume;

/*
 * support audio formats
 */
enum dd_audio_format {
	DD_AUDIO_FORMAT_WAV,
};

/* sound element
 * meant for small-size sound effects
 */
struct dd_sound {
	char filename[400];
	char filenameW[400];
	#ifndef AVDL_DIRECT3D11
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	int index;
	#else
	Mix_Chunk *sound;
	#endif
	#endif
	int playingChannel;

	void (*load)(struct dd_sound *, const char *filename, enum dd_audio_format format);
	void (*clean)(struct dd_sound *);
	void (*play)(struct dd_sound *);
	void (*playLoop)(struct dd_sound *, int loops);
	void (*stop)(struct dd_sound *);
};

void dd_sound_create(struct dd_sound *);
void dd_sound_load(struct dd_sound *, const char *filename, enum dd_audio_format format);
void dd_sound_clean(struct dd_sound *);
void dd_sound_play(struct dd_sound *);
void dd_sound_playLoop(struct dd_sound *, int loops);
void dd_sound_stop(struct dd_sound *);

void avdl_sound_setVolume(int volume);
int avdl_sound_getVolume();

int avdl_audio_initialise();
int avdl_audio_deinitialise();

#ifdef __cplusplus
}
#endif

#endif
