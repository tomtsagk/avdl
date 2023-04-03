#include "dd_sound.h"
#include "dd_log.h"
#include <string.h>
#include "dd_game.h"
#include "avdl_assetManager.h"

#if DD_PLATFORM_ANDROID
#include <jni.h>
extern jclass *clazz;
extern JavaVM *jvm;
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

	#ifndef AVDL_DIRECT3D11
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
	#ifndef AVDL_DIRECT3D11
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
	#ifndef AVDL_DIRECT3D11
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

void dd_sound_play(struct dd_sound *o) {
	#ifndef AVDL_DIRECT3D11
	if (!dd_hasAudio) return;
	if (avdl_sound_volume <= 1) return;
	#if DD_PLATFORM_ANDROID
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			dd_log("avdl: failed to attach thread for new world");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	// get string from asset (in java)
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*env)->NewStringUTF(env, o->filename);
	jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter, 0);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, 0);
	#endif
	#endif
}

void dd_sound_playLoop(struct dd_sound *o, int loops) {
	#ifndef AVDL_DIRECT3D11
	if (!dd_hasAudio) return;
	if (avdl_sound_volume <= 1) return;
	#if DD_PLATFORM_ANDROID
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			dd_log("avdl: failed to attach thread for new world");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	// get string from asset (in java)
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*env)->NewStringUTF(env, o->filename);
	jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter, 1);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, loops);
	#endif
	#endif
}

void dd_sound_stop(struct dd_sound *o) {
	#ifndef AVDL_DIRECT3D11
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	if (o->index == -1) return;
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			dd_log("avdl: failed to attach thread for new world");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	// get string from asset (in java)
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "StopAudio", "(I)V");
	jint *parameter = o->index;
	(*(*env)->CallStaticVoidMethod)(env, clazz, MethodID, parameter);

	o->index = -1;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	Mix_HaltChannel(o->playingChannel);
	#endif
	#endif
}

void avdl_sound_setVolume(int volume) {
	#ifndef AVDL_DIRECT3D11
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
	#ifndef AVDL_DIRECT3D11
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
