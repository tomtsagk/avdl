#include "avdl_ads.h"
#include <string.h>
#include "avdl_log.h"

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <jni.h>
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass mainActivityp;
extern jmethodID loadFullscreenAdMethodId;
extern jmethodID showFullscreenAdMethodId;
extern jmethodID loadRewardedAdMethodId;
extern jmethodID showRewardedAdMethodId;
#endif

static int hasReward = 0;
static int rewardAmount = 0;
static char rewardType[100];

int avdl_ads_enabled() {
	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	return 1;
	#else
	return 0;
	#endif
}

void avdl_ads_loadFullScreenAd() {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &jniEnv, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	(*(*jniEnv)->CallVoidMethod)(jniEnv, mainActivityp, loadFullscreenAdMethodId);

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

}

void avdl_ads_showFullScreenAd() {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &jniEnv, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	(*(*jniEnv)->CallVoidMethod)(jniEnv, mainActivityp, showFullscreenAdMethodId);

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

}

void avdl_ads_loadRewardedAd() {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &jniEnv, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	(*(*jniEnv)->CallVoidMethod)(jniEnv, mainActivityp, loadRewardedAdMethodId);

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

}

void avdl_ads_showRewardedAd() {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &jniEnv, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	hasReward = 0;
	(*(*jniEnv)->CallVoidMethod)(jniEnv, mainActivityp, showRewardedAdMethodId);

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

}

void avdl_ads_onRewardedAd(int amount, const char *type) {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	if (strlen(type) >= 100) {
		avdl_log("avdl error: ad reward type can have a maximum of 100 characters: %s", type);
		return;
	}

	hasReward = 1;
	rewardAmount = amount;
	strcpy(rewardType, type);
	#endif
}

int avdl_ads_hasReward() {
	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	return hasReward;
	#else
	return 0;
	#endif
}

int avdl_ads_getRewardAmount() {
	return rewardAmount;
}

void avdl_ads_consumeReward() {
	hasReward = 0;
}

int avdl_ads_isRewardTypeEqualTo(const char *to) {
	return strcmp(rewardType, to) == 0;
}
