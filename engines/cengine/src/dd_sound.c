#include "dd_sound.h"
#include "dd_log.h"
#include <string.h>

#if DD_PLATFORM_ANDROID
#include <jni.h>
extern jclass *clazz;
extern JNIEnv *jniEnv;
extern JavaVM *jvm;
#else
#include <SDL2/SDL_mixer.h>
#endif

int dd_hasAudio = 1;

void dd_sound_create(struct dd_sound *o) {
	o->load = dd_sound_load;
	o->clean = dd_sound_clean;
	o->play = dd_sound_play;
	o->playLoop = dd_sound_playLoop;
	o->stop = dd_sound_stop;
	o->setVolume = dd_sound_setVolume;

	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	o->filename[0] = '\0';
	o->index = -1;
	#else
	o->sound = 0;
	#endif

	o->playingChannel = -1;
}

void dd_sound_load(struct dd_sound *o, const char *filename, enum dd_audio_format format) {
	if (!dd_hasAudio) return;
	if (strlen(filename) >= 100) {
		dd_log("avdl: asset name can't be more than 100 characters: %s", filename);
	}
	#if DD_PLATFORM_ANDROID
	strcpy(o->filename, filename);
	#else
	o->sound = Mix_LoadWAV(filename);
	#endif
}

void dd_sound_clean(struct dd_sound *o) {
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	#else
	if (o->sound) {
		Mix_FreeChunk(o->sound);
		o->sound = 0;
	}
	#endif
}

void dd_sound_play(struct dd_sound *o) {
	if (!dd_hasAudio) return;
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
	jniEnv = env;

	// get string from asset (in java)
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*jniEnv)->NewStringUTF(jniEnv, o->filename);
	jint result = (jint)(*(*jniEnv)->CallStaticIntMethod)(jniEnv, clazz, MethodID, parameter, 0);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, 0);
	#endif
}

void dd_sound_playLoop(struct dd_sound *o, int loops) {
	if (!dd_hasAudio) return;
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
	jniEnv = env;

	// get string from asset (in java)
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*jniEnv)->NewStringUTF(jniEnv, o->filename);
	jint result = (jint)(*(*jniEnv)->CallStaticIntMethod)(jniEnv, clazz, MethodID, parameter, 1);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	o->playingChannel = Mix_PlayChannel(-1, o->sound, loops);
	#endif
}

void dd_sound_stop(struct dd_sound *o) {
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
	jniEnv = env;

	// get string from asset (in java)
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "StopAudio", "(I)V");
	jint *parameter = o->index;
	(*(*jniEnv)->CallStaticVoidMethod)(jniEnv, clazz, MethodID, parameter);

	o->index = -1;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	Mix_HaltChannel(o->playingChannel);
	#endif
}
void dd_sound_setVolume(struct dd_sound *o, int volume) {
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	#else
	if (volume < 0) volume = 0;
	if (volume > 100) volume = 100;
	Mix_Volume(o->playingChannel, volume *(MIX_MAX_VOLUME /100.0));
	#endif
}
