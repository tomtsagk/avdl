#include "dd_music.h"
#include "dd_log.h"
#include <string.h>
#include "dd_game.h"

#if DD_PLATFORM_ANDROID
#include <jni.h>
extern jclass *clazz;
extern JavaVM *jvm;
#endif

void dd_music_create(struct dd_music *o) {
	o->load = dd_music_load;
	o->clean = dd_music_clean;
	o->play = dd_music_play;
	o->playLoop = dd_music_playLoop;
	o->stop = dd_music_stop;

	if (!dd_hasAudio) return;
	o->filename[0] = '\0';
	#if DD_PLATFORM_ANDROID
	o->index = -1;
	#else
	o->music = 0;
	#endif

}

void dd_music_load(struct dd_music *o, const char *filename, int type) {
	if (!dd_hasAudio) return;
	if (strlen(filename) >= 100) {
		dd_log("avdl: asset name can't be more than 100 characters: %s", filename);
	}
	#if DD_PLATFORM_ANDROID
	strcpy(o->filename, filename);
	#else

	#if defined(_WIN32) || defined(WIN32)
	wcscpy(o->filenameW, avdl_getProjectLocation());
	mbstowcs((o->filenameW +wcslen(o->filenameW)), filename, 400 -wcslen(o->filenameW));
	//wprintf(L"add assetW: %lS\n", meshToLoad.filenameW);
	#else
	strcpy(o->filename, avdl_getProjectLocation());
	strcat(o->filename, filename);
	o->music = Mix_LoadMUS(o->filename);
	if (!o->music) {
		dd_log("avdl: error playing dd_music: '%s': %s", filename, Mix_GetError());
	}
	#endif

	#endif
}

void dd_music_clean(struct dd_music *o) {
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	#else
	if (o->music) {
		Mix_FreeMusic(o->music);
		o->music = 0;
	}
	#endif
}

void dd_music_play(struct dd_music *o) {
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

	// get string from asset (in java)
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*env)->NewStringUTF(env, o->filename);
	jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter, 0);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	Mix_PlayMusic(o->music, 1);
	#endif
}

void dd_music_playLoop(struct dd_music *o, int loops) {
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

	// get string from asset (in java)
	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "PlayAudio", "(Ljava/lang/String;I)I");
	jstring *parameter = (*env)->NewStringUTF(env, o->filename);
	jint result = (jint)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter, 1);
	o->index = result;

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else
	Mix_PlayMusic(o->music, -1);
	#endif
}

void dd_music_stop(struct dd_music *o) {
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
	Mix_HaltMusic();
	#endif
}

void avdl_music_setVolume(int volume) {
	if (!dd_hasAudio) return;
	#if DD_PLATFORM_ANDROID
	#else
	if (volume < 0) volume = 0;
	if (volume > 100) volume = 100;
	Mix_VolumeMusic((volume /100.0) *MIX_MAX_VOLUME);
	#endif
}

int avdl_music_getVolume() {
	if (!dd_hasAudio) return 0;
	#if DD_PLATFORM_ANDROID
	return 0;
	#else
	int volume = Mix_VolumeMusic(-1);
	return ((float) volume /MIX_MAX_VOLUME) *100;
	#endif
}
