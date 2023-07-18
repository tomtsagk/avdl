#include "avdl_ads.h"

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <jni.h>
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass mainActivityp;
extern jmethodID loadFullscreenAdMethodId;
extern jmethodID showFullscreenAdMethodId;
#endif

void avdl_ads_loadFullScreenAd() {

	#if defined( AVDL_ANDROID ) && defined( AVDL_ADMOB )
	JNIEnv *env;
	int getEnvStat = (*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &jniEnv, NULL) != 0) {
			dd_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
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
			dd_log("avdl: failed to attach thread for ads");
		}
	} else if (getEnvStat == JNI_OK) {
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	(*(*jniEnv)->CallVoidMethod)(jniEnv, mainActivityp, showFullscreenAdMethodId);

	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

}
