#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <string.h>
#include "avdl_localisation.h"

// world interface and starting world
#include "avdl_cengine.h"

extern enum avdl_locale AVDL_LOCALE_CURRENT;

#ifdef AVDL_DIRECT3D11
#elif defined(AVDL_WINDOWS)
HANDLE updateDrawMutex;
#else

#ifdef AVDL_DIRECT3D11
#else
#include <unistd.h>
#endif

#ifdef AVDL_DIRECT3D11
#else
// Threads
#include <pthread.h>
pthread_t updatePthread;
pthread_mutex_t updateDrawMutex;
#endif

#endif

/*
 * android includes
 */
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )

	#include <jni.h>
	jclass *clazz;
	jclass mainActivityp;

	jmethodID BitmapMethodId;
	jmethodID PlayAudioMethodId;
	jmethodID StopAudioMethodId;

	jmethodID loadFullscreenAdMethodId;
	jmethodID showFullscreenAdMethodId;

	jmethodID loadRewardedAdMethodId;
	jmethodID showRewardedAdMethodId;

	// Reconstruct engine to use one JNIEnv per world
	JNIEnv *jniEnv;
	JavaVM* jvm = 0;
	jobject activity = 0;

	pthread_mutex_t jniMutex;

	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>

	AAssetManager *aassetManager;

/*
 * cengine includes
 */
#elif defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	// curl
	//#include <curl/curl.h>

	#include "dd_async_call.h"

#endif

/*
 * general event functions
 */
void onResume();
void onPause();

#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
// threads
//pthread_mutex_t asyncCallMutex;
#endif

int avdl_state_initialised = 0;
int avdl_use_default_locale = 1;

extern struct avdl_engine engine;

int dd_main(int argc, char *argv[]) {

	#ifdef AVDL_DIRECT3D11
	#else
	/*
	 * parse command line arguments
	 */
	for (int i = 0; i < argc; i++) {

		// custom save directory (for specific platforms)
		if (strcmp(argv[i], "--avdl-save-dir") == 0) {
			i++;
			if (i >= argc) {
				avdl_log("avdl: please provide a save directory after \"--avdl-save-dir\"");
				return -1;
			}
			strcpy(avdl_data_saveDirectory, argv[i]);
		}
		else
		// set locale
		if (strcmp(argv[i], "--locale") == 0) {
			i++;
			if (i >= argc) {
				avdl_log("avdl: please provide a language, for example 'en'");
				return -1;
			}
			avdl_locale_set(argv[i]);
			avdl_use_default_locale = 0;
		}
		else
		// verify game, for testing reasons
		if (strcmp(argv[i], "--verify") == 0) {
			engine.verify = 1;
		}
		else
		// quiet mode
		if (strcmp(argv[i], "-q") == 0) {
			engine.quiet = 1;
		}

	}
	#endif

	#if defined( AVDL_ANDROID )
	// initialise pthread mutex for jni
	if (pthread_mutex_init(&jniMutex, NULL) != 0)
	{
		avdl_log("avdl: mutex for jni init failed");
		return -1;
	}
	#endif

	#ifdef AVDL_DIRECT3D11
	#elif defined(AVDL_WINDOWS)
	updateDrawMutex = CreateMutex(NULL, FALSE, NULL);
	#else
	if (pthread_mutex_init(&updateDrawMutex, NULL) != 0)
	{
		avdl_log("avdl: mutex for update/draw init failed");
		return -1;
	}
	#endif

	// initialise engine
	if (avdl_engine_init(&engine)) {
		avdl_log("avdl: error initialising engine");
		return -1;
	}
	avdl_engine_initWorld(&engine, dd_default_world_constructor, dd_default_world_size);
	avdl_state_initialised = 1;

	onResume();

	// on windows and linux this is the game loop
	// android handles the loop differently
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	// keeps running until game exits
	avdl_engine_loop(&engine);

	avdl_engine_clean(&engine);

	#ifdef AVDL_DIRECT3D11
	#elif defined(_WIN32) || defined(WIN32)
	CloseHandle(updateDrawMutex);
	#else
	pthread_mutex_destroy(&updateDrawMutex);
	#endif

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	//curl_global_cleanup();
	#endif

	#endif

	// everything ok
	return 0;
}

// define main when not in unit tests or cpp mode
#ifndef AVDL_DIRECT3D11
#ifndef AVDL_UNIT_TEST
#ifndef AVDL_STEAM
int main(int argc, char *argv[]) {
	return dd_main(argc, argv);
}
#endif
#endif
#endif

#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
struct dd_async_call dd_asyncCall = {0};
int dd_isAsyncCallActive = 0;
#endif

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
void updateThread();
#endif

void onResume() {

	if (!avdl_state_initialised) return;

	#ifdef AVDL_DIRECT3D11
	#elif defined(AVDL_WINDOWS)
	WaitForSingleObject(updateDrawMutex, INFINITE);
	#else
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	dd_flag_exit = 0;
	dd_flag_focused = 1;
	#ifdef AVDL_DIRECT3D11
	#elif defined(AVDL_WINDOWS)
	ReleaseMutex(updateDrawMutex);
	#else
	pthread_mutex_unlock(&updateDrawMutex);
	#endif

	if (avdl_engine_isPaused(&engine)) {
		avdl_engine_setPaused(&engine, 0);
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		pthread_create(&updatePthread, NULL, &updateThread, NULL);
		#elif defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		// resume update with sdl?
		#endif
	}
}

void onPause() {

	/*
	if (nworld_constructor) {
		avdl_log("cancel new world thread");
		pthread_cancel(newWorldPthread);
		nworld_constructor = 0;
		avdl_log("canceled new world thread");
	}
	*/

	if (!avdl_state_initialised) return;

	if (!avdl_engine_isPaused(&engine)) {
		avdl_engine_setPaused(&engine, 1);
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		pthread_join(updatePthread, NULL);
		#endif
	}
	#ifdef AVDL_DIRECT3D11
	#elif defined(AVDL_WINDOWS)
	WaitForSingleObject(updateDrawMutex, INFINITE);
	#else
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	dd_flag_focused = 0;
	//dd_flag_initialised = 0;
	#ifdef AVDL_DIRECT3D11
	#elif defined(AVDL_WINDOWS)
	ReleaseMutex(updateDrawMutex);
	#else
	pthread_mutex_unlock(&updateDrawMutex);
	#endif
}

/*
 * Android specific functions that call the engine's events
 */
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )

void updateThread() {
	while (!dd_flag_exit && !avdl_engine_isPaused(&engine)) {
		pthread_mutex_lock(&updateDrawMutex);
		avdl_engine_update(&engine, 1);
		if (dd_flag_exit) {
			/*
			JNIEnv *env;
			int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);

			if (getEnvStat == JNI_EDETACHED) {
				if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
					avdl_log("avdl: failed to attach thread for new world");
				}
			} else if (getEnvStat == JNI_OK) {
			} else if (getEnvStat == JNI_EVERSION) {
				avdl_log("avdl: GetEnv: version not supported");
			}
			jniEnv = env;
			avdl_log("get method");
			jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "CloseApplication", "()V");
			avdl_log("call method");
			(*(*jniEnv)->CallStaticVoidMethod)(jniEnv, clazz, MethodID);
			avdl_log("detach");
			if (getEnvStat == JNI_EDETACHED) {
				(*jvm)->DetachCurrentThread(jvm);
			}
			*/
			#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			if (!avdl_state_initialised) return;
			avdl_state_initialised = 0;
			avdl_engine_clean(&engine);
			pthread_mutex_destroy(&jniMutex);
			// potentially need this
			//pthread_mutex_unlock(&updateDrawMutex);
			pthread_mutex_destroy(&updateDrawMutex);
			exit(0);
			#endif
		}
		pthread_mutex_unlock(&updateDrawMutex);
		usleep(engine.avdl_fps_delay *1000);
	}
}

/*
 * nativeInit : Called when the app is first created
 */
void Java_dev_afloof_avdl_AvdlRenderer_nativeInit(JNIEnv* env, jobject thiz, jobject mainActivity) {

	#if defined(AVDL_QUEST2)
	// initialise pthread mutex for jni
	if (pthread_mutex_init(&jniMutex, NULL) != 0)
	{
		avdl_log("avdl: mutex for jni init failed");
		return;
	}
	#endif

	pthread_mutex_lock(&jniMutex);
	// Global variables to access Java virtual machine and environment
	(*env)->GetJavaVM(env, &jvm);
	(*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);
	jclass classLocal = (*(*jniEnv)->FindClass)(jniEnv, "dev/afloof/avdl/AvdlActivity");
	clazz = (*jniEnv)->NewGlobalRef(jniEnv, classLocal);
	mainActivityp = (*jniEnv)->NewGlobalRef(jniEnv, mainActivity);

	BitmapMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "ReadBitmap", "(Ljava/lang/String;)[Ljava/lang/Object;");
	PlayAudioMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "PlayAudio", "(Ljava/lang/String;II)I");
	StopAudioMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "StopAudio", "(I)V");

	loadFullscreenAdMethodId = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "loadFullscreenAd", "(I)V");
	showFullscreenAdMethodId = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "showFullscreenAd", "(I)V");

	loadRewardedAdMethodId = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "loadRewardedAd", "(I)V");
	showRewardedAdMethodId = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "showRewardedAd", "(I)V");

	// grab internal save path, for save/load functionality
	jmethodID getFilesDir = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "getFilesDir", "()Ljava/io/File;");
	jobject dirobj = (*(*jniEnv)->CallObjectMethod)(jniEnv, mainActivity,getFilesDir);
	jclass dir = (*(*jniEnv)->GetObjectClass)(jniEnv, dirobj);
	jmethodID getStoragePath =
		(*(*jniEnv)->GetMethodID)(jniEnv, dir, "getAbsolutePath", "()Ljava/lang/String;");
	jstring path=(jstring)(*(*jniEnv)->CallObjectMethod)(jniEnv, dirobj, getStoragePath);
	const char *pathstr=(*(*jniEnv)->GetStringUTFChars)(jniEnv, path, 0);
	strcpy(avdl_data_saveDirectory, pathstr);
	(*(*jniEnv)->ReleaseStringUTFChars)(jniEnv, path, pathstr);
	pthread_mutex_unlock(&jniMutex);

	// engine is not initialised - start everything from scratch
	if (!avdl_state_initialised) {
		// Initialises the engine and the first world
		dd_main(0, 0);
	}
	// engine is initialised - refresh graphics context
	else {
		avdl_graphics_generateContext();
	}

}

/*
 * nativeDone : Called when closing the app
 */
void Java_dev_afloof_avdl_AvdlActivity_nativeDone(JNIEnv*  env) {
	if (dd_flag_exit == 0) {
		pthread_mutex_lock(&jniMutex);
		jniEnv = 0;
		jvm = 0;
		pthread_mutex_unlock(&jniMutex);
	}
}

/*
 * nativeResize : Called when screen size changes
 */
void Java_dev_afloof_avdl_AvdlRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
	dd_width = w;
	dd_height = h;

	avdl_engine_resize(&engine, w, h);
}

/*
 * nativeRender : Called every so often to draw a frame
 */
void Java_dev_afloof_avdl_AvdlRenderer_nativeRender(JNIEnv* env) {
	pthread_mutex_lock(&updateDrawMutex);
	if (!dd_flag_exit) {
		avdl_engine_draw(&engine);
	}
	pthread_mutex_unlock(&updateDrawMutex);
}

/*
 * nativeMouseInput* : Called when a mouse/touch event happens, this is asynchronous
 * 	but will set a flag that the engine can pick up when ready
 */
void Java_dev_afloof_avdl_AvdlGLSurfaceView_nativeMouseInputDown(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	avdl_input_AddInputLocation(&engine.input, AVDL_INPUT_MOUSE_LEFT, AVDL_INPUT_STATE_DOWN, mouseX, mouseY);
}

void Java_dev_afloof_avdl_AvdlGLSurfaceView_nativeMouseInputUp(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	avdl_input_AddInputLocation(&engine.input, AVDL_INPUT_MOUSE_LEFT, AVDL_INPUT_STATE_UP, mouseX, mouseY);
}

void Java_dev_afloof_avdl_AvdlGLSurfaceView_nativeMouseInputMove(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	avdl_input_AddInputLocation(&engine.input, AVDL_INPUT_MOUSE_LEFT, AVDL_INPUT_STATE_MOVE, mouseX, mouseY);
}

void Java_dev_afloof_avdl_AvdlGLSurfaceView_nativeTogglePauseResume(JNIEnv* env) {
}

void Java_dev_afloof_avdl_AvdlActivity_nativePause(JNIEnv* env) {
	if (dd_flag_exit == 0) {
		onPause();
	}
}

void Java_dev_afloof_avdl_AvdlActivity_nativeResume(JNIEnv* env) {
	if (dd_flag_exit == 0) {
		onResume();
	}
}

void Java_dev_afloof_avdl_AvdlActivity_nativeKeyDown(JNIEnv*  env, jobject obj, jint key) {
	if (dd_flag_exit == 0) {
		pthread_mutex_lock(&updateDrawMutex);
		avdl_input_AddInput(&engine.input, key, AVDL_INPUT_STATE_DOWN);
		pthread_mutex_unlock(&updateDrawMutex);
	}
}

void Java_dev_afloof_avdl_AvdlActivity_nativeOnRewardedAd(JNIEnv* env, jobject thiz, int reward_amount, jstring reward_type) {
	const char *nativeString = (*env)->GetStringUTFChars(env, reward_type, 0);
	avdl_ads_onRewardedAd(reward_amount, nativeString);
	(*env)->ReleaseStringUTFChars(env, reward_type, nativeString);
}

#if defined(AVDL_QUEST2)
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	//avdl_log("JNI_OnLoad");
	jvm = vm;
	(*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_6);
	jclass classLocal = (*(*jniEnv)->FindClass)(jniEnv, "dev/afloof/avdl/AvdlActivity");
	clazz = (*jniEnv)->NewGlobalRef(jniEnv, classLocal);

	BitmapMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "ReadBitmap", "(Ljava/lang/String;)[Ljava/lang/Object;");
	PlayAudioMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "PlayAudio", "(Ljava/lang/String;II)I");
	StopAudioMethodId = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "StopAudio", "(I)V");

	if (pthread_mutex_init(&updateDrawMutex, NULL) != 0)
	{
		avdl_log("avdl: mutex for update/draw init failed");
		return JNI_VERSION_1_6;
	}

	return JNI_VERSION_1_6;
}

void set_android_save_dir(jobject activity) {
	// grab internal save path, for save/load functionality
	jmethodID getFilesDir = (*(*jniEnv)->GetMethodID)(jniEnv, clazz, "getFilesDir", "()Ljava/io/File;");
	jobject dirobj = (*(*jniEnv)->CallObjectMethod)(jniEnv, activity, getFilesDir);
	jclass dir = (*(*jniEnv)->GetObjectClass)(jniEnv, dirobj);
	jmethodID getStoragePath =
		(*(*jniEnv)->GetMethodID)(jniEnv, dir, "getAbsolutePath", "()Ljava/lang/String;");
	jstring path=(jstring)(*(*jniEnv)->CallObjectMethod)(jniEnv, dirobj, getStoragePath);
	const char *pathstr=(*(*jniEnv)->GetStringUTFChars)(jniEnv, path, 0);
	strcpy(avdl_data_saveDirectory, pathstr);
	//avdl_log("save dir: %s", avdl_data_saveDirectory);
	(*(*jniEnv)->ReleaseStringUTFChars)(jniEnv, path, pathstr);
}
#endif

void Java_dev_afloof_avdl_AvdlActivity_nativeSetAssetManager(JNIEnv* env, jobject thiz, jobject assetManager) {
	aassetManager = AAssetManager_fromJava(env, assetManager);
}

void Java_dev_afloof_avdl_AvdlActivity_nativeSetLocale(JNIEnv* env, jobject thiz, jint locale) {
	int loc = locale;
	// for now, check languages manually
	switch (loc) {
		case 0: avdl_locale_set("en"); break;
		case 1: avdl_locale_set("de"); break;
		case 2: avdl_locale_set("ja"); break;
		case 3: avdl_locale_set("el"); break;
	}
}

#endif
