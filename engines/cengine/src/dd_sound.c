#include "dd_sound.h"
#include "avdl_log.h"
#include <string.h>
#include "dd_game.h"
#include "avdl_assetManager.h"

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <jni.h>
extern jclass *clazz;
extern JavaVM *jvm;
extern jmethodID PlayAudioMethodId;
extern jmethodID StopAudioMethodId;
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
static pthread_t soundThread = 0;
#endif

int dd_hasAudio = 1;
int dd_numberOfAudioChannels = 0;

int avdl_sound_volume = 100;

void dd_sound_create(struct dd_sound *o) {
	o->load = dd_sound_load;
	o->clean = dd_sound_clean;
	o->play = dd_sound_play;
	o->playLoop = dd_sound_playLoop;
	o->stop = dd_sound_stop;

	#if !defined( AVDL_DIRECT3D11 )
	if (!dd_hasAudio) return;
	o->filename[0] = '\0';
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	o->index = -1;
	#else
	o->sound = 0;
	#endif

	o->playingChannel = -1;
	#endif
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
static int id_generator = 1;
#endif

void dd_sound_load(struct dd_sound *o, const char *filename, enum dd_audio_format format) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	if (strlen(filename) >= 100) {
		avdl_log("avdl: asset name can't be more than 100 characters: %s", filename);
	}
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(o->filename, filename);
	o->index = id_generator;
	id_generator++;
	if (id_generator >= 65000) {
		id_generator = 1;
	}
	#else

	#if defined(_WIN32) || defined(WIN32)
	strcpy(o->filename, filename);
	#else
	strcpy(o->filename, avdl_getProjectLocation());
	strcat(o->filename, GAME_ASSET_PREFIX);
	strcat(o->filename, filename);
	#endif
	o->sound = Mix_LoadWAV(o->filename);
	if (!o->sound) {
		avdl_log("avdl: error playing dd_sound: '%s': %s", filename, Mix_GetError());
	}

	#endif
	#endif
}

void dd_sound_clean(struct dd_sound *o) {
	#if !defined( AVDL_DIRECT3D11 )
	if (!dd_hasAudio) return;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else
	if (o->sound) {
		Mix_FreeChunk(o->sound);
		o->sound = 0;
	}
	#endif
	#endif
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
static int is_thread_waiting = 0;
static pthread_cond_t cond;
static pthread_mutex_t mutex;

static char playingAudio[150];
static int playingAudioId = 0;
static int isLooping = 0;
static int stoppingAudioId = 0;

#define AUDIO_MAX 10
static char playingAudioQueue[AUDIO_MAX][150];
static int playingAudioIdQueue[AUDIO_MAX];
static int playingAudioLoopQueue[AUDIO_MAX];
static int playingAudioCount = 0;
static int stoppingAudioIdQueue[AUDIO_MAX];
static int stoppingAudioCount = 0;

static void *play_sound_thread_function(void *data) {

	int keep_sound_thread = 1;
	while (keep_sound_thread) {
		JNIEnv *env;
		int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_6);

		if (getEnvStat == JNI_EDETACHED) {
			if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
				avdl_log("avdl: failed to attach thread for new world");
			}
		} else if (getEnvStat == JNI_OK) {
		} else if (getEnvStat == JNI_EVERSION) {
			avdl_log("avdl: GetEnv: version not supported");
		}

		if (playingAudioId) {
			// get string from asset (in java)
			jstring *parameter = (*env)->NewStringUTF(env, playingAudio);
			jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, PlayAudioMethodId, parameter, isLooping, playingAudioId);
			playingAudioId = 0;
		}

		if (stoppingAudioId) {
			// get string from asset (in java)
			(*(*env)->CallStaticVoidMethod)(env, clazz, StopAudioMethodId, stoppingAudioId);
			stoppingAudioId = 0;
		}

		if (getEnvStat == JNI_EDETACHED) {
			(*jvm)->DetachCurrentThread(jvm);
		}

		pthread_mutex_lock(&mutex);

		if (playingAudioCount > 0) {
			strcpy(playingAudio, playingAudioQueue[playingAudioCount-1]);
			playingAudioId = playingAudioIdQueue[playingAudioCount-1];
			isLooping = playingAudioLoopQueue[playingAudioCount-1];
			playingAudioCount--;
		}

		if (stoppingAudioCount > 0) {
			stoppingAudioId = stoppingAudioIdQueue[stoppingAudioCount-1];
			stoppingAudioCount--;
		}

		if (playingAudioId > 0 || stoppingAudioId > 0) {
			pthread_mutex_unlock(&mutex);
			continue;
		}

		// wait for new audio to come
		is_thread_waiting = 1;
		pthread_cond_wait(&cond, &mutex);

		// reset pthread condition
		pthread_cond_destroy(&cond);
		pthread_cond_init(&cond, 0);

		pthread_mutex_unlock(&mutex);

	}

	return 0;
}

static int is_thread_running = 0;
#endif

void dd_sound_play(struct dd_sound *o) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	if (avdl_sound_volume <= 1) return;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )

	if (avdl_sound_volume <= 1) return;

	// no thread active - create one and play audio
	if (!is_thread_running) {
		is_thread_running = 1;
		strcpy(playingAudio, o->filename);
		playingAudioId = o->index;
		isLooping = 0;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, 0);
		pthread_create(&soundThread, NULL, play_sound_thread_function, 0);
		pthread_detach(soundThread); // do not wait for thread result code
	}
	// thread active - play audio on it
	else {
		pthread_mutex_lock(&mutex);

		// thread is waiting for next audio - give it one
		if (is_thread_waiting) {
			strcpy(playingAudio, o->filename);
			playingAudioId = o->index;
			isLooping = 0;
			is_thread_waiting = 0;
			pthread_cond_signal(&cond);
		}
		// thread is busy, potentially queue next sound
		else {
			if (playingAudioCount < AUDIO_MAX) {
				strcpy(playingAudioQueue[playingAudioCount], o->filename);
				playingAudioIdQueue[playingAudioCount] = o->index;
				playingAudioLoopQueue[playingAudioCount] = 0;
				playingAudioCount++;
			}
			// too many sounds in queue - should never happen
			else {
			}
		}
		pthread_mutex_unlock(&mutex);
	}

	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, 0);
	#endif
	#endif
}

void dd_sound_playLoop(struct dd_sound *o, int loops) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	if (avdl_sound_volume <= 1) return;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	if (avdl_sound_volume <= 1) return;

	// no thread active - create one and play audio
	if (!is_thread_running) {
		is_thread_running = 1;
		strcpy(playingAudio, o->filename);
		playingAudioId = o->index;
		isLooping = 1;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, 0);
		pthread_create(&soundThread, NULL, play_sound_thread_function, 0);
		pthread_detach(soundThread); // do not wait for thread result code
	}
	// thread active - play audio on it
	else {
		pthread_mutex_lock(&mutex);

		// thread is waiting for next audio - give it one
		if (is_thread_waiting) {
			strcpy(playingAudio, o->filename);
			playingAudioId = o->index;
			isLooping = 1;
			is_thread_waiting = 0;
			pthread_cond_signal(&cond);
		}
		// thread is busy, potentially queue next sound
		else {
			if (playingAudioCount < AUDIO_MAX) {
				strcpy(playingAudioQueue[playingAudioCount], o->filename);
				playingAudioIdQueue[playingAudioCount] = o->index;
				playingAudioLoopQueue[playingAudioCount] = 1;
				playingAudioCount++;
			}
			// too many sounds in queue - should never happen
			else {
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, loops);
	#endif
	#endif
}

void dd_sound_stop(struct dd_sound *o) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )

	// no thread active - create one and play audio
	if (!is_thread_running) {
		is_thread_running = 1;
		stoppingAudioId = o->index;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, 0);
		pthread_create(&soundThread, NULL, play_sound_thread_function, 0);
		pthread_detach(soundThread); // do not wait for thread result code
	}
	// thread active - play audio on it
	else {
		pthread_mutex_lock(&mutex);

		// thread is waiting for next audio - give it one
		if (is_thread_waiting) {
			stoppingAudioId = o->index;
			is_thread_waiting = 0;
			pthread_cond_signal(&cond);
		}
		// thread is busy, potentially queue next sound
		else {
			if (stoppingAudioCount < AUDIO_MAX) {
				stoppingAudioIdQueue[stoppingAudioCount] = o->index;
				stoppingAudioCount++;
			}
			// too many sounds in queue - should never happen
			else {
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	#else
	Mix_HaltChannel(o->playingChannel);
	#endif
	#endif
}

void avdl_sound_setVolume(int volume) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	if (volume < 0) volume = 0;
	if (volume > 100) volume = 100;
	avdl_sound_volume = volume;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else
	Mix_Volume(-1, (avdl_sound_volume /100.0) *MIX_MAX_VOLUME);
	#endif
	#endif
}

int avdl_sound_getVolume() {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return 0;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	return avdl_sound_volume;
	#else
	int volume = Mix_Volume(-1, -1);
	return ((float) volume /MIX_MAX_VOLUME) *100;
	#endif
	#endif
	return 0;
}

int avdl_audio_initialise() {

	#if defined(AVDL_DIRECT3D11)
	#else
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else
	/*
	 * initialise audio, if it fails, don't play audio at all
	 * during the game
	 */
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
		avdl_log("avdl: error initialising audio mixer");
		dd_hasAudio = 0;
		return -1;
	}
	// audio initialisation succeeded
	else {
		Mix_Init(MIX_INIT_OGG);

		// make sure there's at least 8 channels
		dd_numberOfAudioChannels = Mix_AllocateChannels(-1);
		if (dd_numberOfAudioChannels < 8) {
			dd_numberOfAudioChannels = Mix_AllocateChannels(8);
		}
	}
	#endif
	#endif

	return 0;
}

int avdl_audio_deinitialise() {
	#if defined(AVDL_DIRECT3D11)
	#else
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else
	Mix_Quit();
	#endif
	#endif
	return 0;
}
