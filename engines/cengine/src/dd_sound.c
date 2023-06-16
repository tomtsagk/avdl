#include "dd_sound.h"
#include "dd_log.h"
#include <string.h>
#include "dd_game.h"
#include "avdl_assetManager.h"

#if DD_PLATFORM_ANDROID
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

	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	o->filename[0] = '\0';
	#if DD_PLATFORM_ANDROID
	o->index = -1;
	#else
	o->sound = 0;
	#endif

	o->playingChannel = -1;
	#endif
}

void dd_sound_load(struct dd_sound *o, const char *filename, enum dd_audio_format format) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	if (strlen(filename) >= 100) {
		dd_log("avdl: asset name can't be more than 100 characters: %s", filename);
	}
	#if DD_PLATFORM_ANDROID
	strcpy(o->filename, filename);
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
		dd_log("avdl: error playing dd_sound: '%s': %s", filename, Mix_GetError());
	}

	#endif
	#endif
}

void dd_sound_clean(struct dd_sound *o) {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	#else
	if (o->sound) {
		Mix_FreeChunk(o->sound);
		o->sound = 0;
	}
	#endif
	#endif
}

#if DD_PLATFORM_ANDROID
static int is_thread_waiting = 0;
static pthread_cond_t cond;
static pthread_mutex_t mutex;
static char playingAudio[150];
static int isLooping = 0;

static void *play_sound_thread_function(void *data) {

	int keep_sound_thread = 1;
	while (keep_sound_thread) {
		JNIEnv *env;
		#if defined(AVDL_QUEST2)
		int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_6);
		#else
		int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);
		#endif

		if (getEnvStat == JNI_EDETACHED) {
			if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
				dd_log("avdl: failed to attach thread for new world");
			}
		} else if (getEnvStat == JNI_OK) {
		} else if (getEnvStat == JNI_EVERSION) {
			dd_log("avdl: GetEnv: version not supported");
		}

		// get string from asset (in java)
		jstring *parameter = (*env)->NewStringUTF(env, playingAudio);
		#if defined(AVDL_QUEST2)
		jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, PlayAudioMethodId, parameter, isLooping);
		#else
		jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
		jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter, isLooping);
		#endif
		//o->index = result;

		if (getEnvStat == JNI_EDETACHED) {
			(*jvm)->DetachCurrentThread(jvm);
		}

		pthread_mutex_lock(&mutex);

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
	#if DD_PLATFORM_ANDROID

	if (avdl_sound_volume <= 1) return;

	// no thread active - create one and play audio
	if (!is_thread_running) {
		is_thread_running = 1;
		strcpy(playingAudio, o->filename);
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
			isLooping = 0;
			is_thread_waiting = 0;
			pthread_cond_signal(&cond);
		}
		// thread is busy, potentially queue next sound
		else {
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
	#if DD_PLATFORM_ANDROID
	if (avdl_sound_volume <= 1) return;

	// no thread active - create one and play audio
	if (!is_thread_running) {
		is_thread_running = 1;
		strcpy(playingAudio, o->filename);
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
			isLooping = 1;
			is_thread_waiting = 0;
			pthread_cond_signal(&cond);
		}
		// thread is busy, potentially queue next sound
		else {
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
	#if DD_PLATFORM_ANDROID
	/*
	if (o->index == -1) return;
	JNIEnv *env;
	#if defined(AVDL_QUEST2)
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_6);
	#else
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);
	#endif

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			dd_log("avdl: failed to attach thread for new world");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	// get string from asset (in java)
	jint *parameter = o->index;
	#if defined(AVDL_QUEST2)
	(*(*env)->CallStaticVoidMethod)(env, clazz, StopAudioMethodId, parameter);
	#else
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "StopAudio", "(I)V");
	(*(*env)->CallStaticVoidMethod)(env, clazz, MethodID, parameter);
	#endif

	o->index = -1;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	*/
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
	#if DD_PLATFORM_ANDROID
	#else
	Mix_Volume(-1, (avdl_sound_volume /100.0) *MIX_MAX_VOLUME);
	#endif
	#endif
}

int avdl_sound_getVolume() {
	#if !defined(AVDL_DIRECT3D11)
	if (!dd_hasAudio) return 0;
	#if DD_PLATFORM_ANDROID
	return avdl_sound_volume;
	#else
	int volume = Mix_Volume(-1, -1);
	return ((float) volume /MIX_MAX_VOLUME) *100;
	#endif
	#endif
	return 0;
}
