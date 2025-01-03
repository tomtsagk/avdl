#include "avdl_achievements.h"

#include "avdl_log.h"

#if !defined(AVDL_STEAM) && !defined(AVDL_QUEST2)

#if defined( AVDL_ANDROID )
#include <pthread.h>
#include <unistd.h>
#include <jni.h>
extern pthread_mutex_t updateDrawMutex;
extern pthread_mutex_t jniMutex;
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass *clazz;
extern jmethodID BitmapMethodId;
#endif

struct avdl_achievements *avdl_achievements_create() {
	return 0;
}

void avdl_achievements_clean(struct avdl_achievements *o) {
}

void avdl_achievements_set(struct avdl_achievements *o, const char *achievementId) {

	#if defined( AVDL_ANDROID )
	JNIEnv *env;

	// temporarily have different jni versions for android and quest 2
	// should both be 1.6 once tested
	#if defined(AVDL_QUEST2)
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_6);
	#else
	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);
	#endif

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for new world");
		}
	// thread already attached to jni
	} else if (getEnvStat == JNI_OK) {
	// wrong version
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "UnlockAchievement", "(Ljava/lang/String;)I");
	jstring *parameter = (*env)->NewStringUTF(env, achievementId);
	jobjectArray result = (jstring)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter);

	// success unlocking achievement
	if (result) {
	}
	// unsupported error?
	else {
	}
	#endif
}

void avdl_achievements_unset(struct avdl_achievements *o, const char *achievementId) {
}
#endif
