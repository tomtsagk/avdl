#include "avdl_webapi.h"
#include <string.h>

#if defined(DD_PLATFORM_ANDROID) || defined(AVDL_QUEST2)
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

#if defined(AVDL_OS_WINDOWS)
#include <windows.h>
#include <shellapi.h>
#endif

#if !defined(AVDL_STEAM)
void avdl_webapi_openurl(const char *url) {

	#if defined(AVDL_OS_LINUX) || defined(AVDL_OS_WINDOWS)

	#if defined(AVDL_OS_LINUX)
	const char *opener = "xdg-open ";
	char *final_cmd = malloc(strlen(opener) +strlen(url) +1);
	strcpy(final_cmd, opener);
	strcat(final_cmd, url);
	system(final_cmd);
	free(final_cmd);
	#elif defined(AVDL_OS_WINDOWS)
	//const char *opener = "start ";
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOW);
	#endif

	#elif defined(DD_PLATFORM_ANDROID) || defined(AVDL_QUEST2)

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
			dd_log("avdl: failed to attach thread for new world");
		}
	// thread already attached to jni
	} else if (getEnvStat == JNI_OK) {
	// wrong version
	} else if (getEnvStat == JNI_EVERSION) {
		dd_log("avdl: GetEnv: version not supported");
	}

	jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "OpenUrl", "(Ljava/lang/String;)I");
	jstring *parameter = (*env)->NewStringUTF(env, url);
	jobjectArray result = (jstring)(*(*env)->CallStaticIntMethod)(env, clazz, MethodID, parameter);

	// success opening url
	if (result) {
	}
	// unsupported error?
	else {
	}

	#endif

}
#endif
