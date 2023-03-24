#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <string.h>

// world interface and starting world
#include "avdl_cengine.h"

#if defined(_WIN32) || defined(WIN32)
#else

#include <unistd.h>
// Threads
#include <pthread.h>
pthread_t updatePthread;
pthread_mutex_t updateDrawMutex;

#endif

/*
 * android includes
 */
#if DD_PLATFORM_ANDROID

	#include <jni.h>
	jclass *clazz;

/*
 * cengine includes
 */
#elif DD_PLATFORM_NATIVE

	// audio
	#include "dd_sound.h"
	#include "dd_music.h"

	// curl
	//#include <curl/curl.h>

	#include "dd_async_call.h"

#endif

extern GLuint defaultProgram;
extern GLuint currentProgram;

/*
 * general event functions
 */
void update();
void draw();
void handleResize(int w, int h);
void clean();

void onResume();
void onPause();

/*
 * input handling functions
 */
void handleKeyboardPress(unsigned char key, int x, int y);
void handleMousePress(int button, int state, int x, int y);
void handlePassiveMotion(int x, int y);

unsigned char input_key;
struct AvdlInput avdl_input;

#undef PI
#define PI 3.1415926535897932f

#if DD_PLATFORM_ANDROID
// Reconstruct engine to use one JNIEnv per world
JNIEnv *jniEnv;
JavaVM* jvm = 0;
jobject activity = 0;
#endif

#if DD_PLATFORM_NATIVE
// threads
//pthread_mutex_t asyncCallMutex;
#endif

#if DD_PLATFORM_ANDROID
pthread_mutex_t jniMutex;
#endif

int avdl_state_initialised = 0;

struct avdl_achievements *achievements = 0;

struct avdl_engine engine;

int dd_main(int argc, char *argv[]) {

	/*
	 * parse command line arguments
	 */
	for (int i = 0; i < argc; i++) {

		// custom save directory (for specific platforms)
		if (strcmp(argv[i], "--avdl-save-dir") == 0) {
			i++;
			if (i >= argc) {
				dd_log("avdl: please provide a save directory after \"--avdl-save-dir\"");
				return -1;
			}
			strcpy(avdl_data_saveDirectory, argv[i]);
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

	#ifdef AVDL_STEAM
	if (!engine.verify) {
		if (!avdl_steam_init()) {
			dd_log("avdl: error initialising steam");
			return -1;
		}
	}
	#endif

	achievements = avdl_achievements_create();
	avdl_initProjectLocation();
	avdl_assetManager_init();

	#if defined(_WIN32) || defined(WIN32)
	char *proj_loc = avdl_getProjectLocation();
	if (proj_loc) {
		if (_wchdir(proj_loc) != 0) {
			dd_log("avdl: failed to change directory: %lS", _wcserror(errno));
			return -1;
		}
	}
	else {
		dd_log("avdl error: unable to get project location");
	}
	#endif

	input_key = 0;
	avdl_input_Init(&avdl_input);

	#if DD_PLATFORM_ANDROID
	// initialise pthread mutex for jni
	if (pthread_mutex_init(&jniMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for jni init failed");
		return -1;
	}
	#endif

	#if defined(_WIN32) || defined(WIN32)
	#else
	if (pthread_mutex_init(&updateDrawMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for update/draw init failed");
		return -1;
	}
	#endif

	#if DD_PLATFORM_NATIVE
	/*
	// initialise curl
	curl_global_init(CURL_GLOBAL_ALL);

	if (pthread_mutex_init(&asyncCallMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for async calls init failed");
		return -1;
	}
	*/
	#endif

	avdl_engine_init(&engine);

	// initialise world
	avdl_engine_initWorld(&engine, dd_default_world_constructor, dd_default_world_size);

	avdl_state_initialised = 1;
	onResume();
	#if DD_PLATFORM_NATIVE

	if (!engine.verify) {

		// keeps running until game exits
		//avdl_engine_loop(&engine);

		// start the loop
		int isRunning = 1;
		SDL_Event event;
		while (isRunning && !dd_flag_exit) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						isRunning = 0;
						break;
					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
							handleResize(event.window.data1, event.window.data2);
						}
						break;
					case SDL_MOUSEMOTION:
						handlePassiveMotion(event.motion.x, event.motion.y);
						break;
					case SDL_MOUSEBUTTONDOWN:
						handleMousePress(0, 0, event.motion.x, event.motion.y);
						break;
					case SDL_MOUSEBUTTONUP:
						handleMousePress(0, 1, event.motion.x, event.motion.y);
						break;
					case SDL_KEYDOWN:
						// temporary keyboard controls
						if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
							handleKeyboardPress(27, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_A) {
							handleKeyboardPress(97, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_D) {
							handleKeyboardPress(100, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_W) {
							handleKeyboardPress(119, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_S) {
							handleKeyboardPress(115, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
							handleKeyboardPress(32, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
							handleKeyboardPress(13, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
							handleKeyboardPress(1, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
							handleKeyboardPress(2, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
							handleKeyboardPress(3, 0, 0);
						}
						else
						if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
							handleKeyboardPress(4, 0, 0);
						}
						break;
				}
			}
			update();
			SDL_Delay(33.333);
		}
	}

	if (engine.verify) {

		/*
		//avdl_assetManager_lockLoading();

		// allocate new world and construct it
		nworld = malloc(nworld_size);
		nworld_constructor(nworld);

		// from now on, new assets can be loaded again
		//avdl_assetManager_unlockLoading();

		// Apply the new world
		cworld = nworld;
		nworld = 0;

		// notify the world that it has loaded assets
		cworld->onload(cworld);

		// resize the new world
		if (cworld->resize) {
			cworld->resize(cworld);
		}

		cworld->update(cworld);
		*/

	}

	clean();
	#endif

	// everything ok
	return 0;
}

// define main when not in unit tests or cpp mode
#ifndef AVDL_UNIT_TEST
#ifndef AVDL_STEAM
int main(int argc, char *argv[]) {
	return dd_main(argc, argv);
}
#endif
#endif

// clean leftovers
void clean() {
	/*
	if (!engine.quiet) {
		dd_log("avdl: cleaning data");
	}
	*/
	if (!avdl_state_initialised) return;
	avdl_state_initialised = 0;

	#ifdef AVDL_STEAM
	if (!engine.verify) {
		avdl_steam_shutdown();
	}
	#endif
	avdl_achievements_clean(achievements);
	avdl_cleanProjectLocation();

	avdl_engine_clean(&engine);

	#if DD_PLATFORM_ANDROID
	pthread_mutex_destroy(&jniMutex);
	#endif

	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_destroy(&updateDrawMutex);
	#endif

	#if DD_PLATFORM_NATIVE
	//curl_global_cleanup();
	#endif
	/*
	if (!engine.quiet) {
		dd_log("avdl: done cleaning data");
	}
	*/
}

// handle resize with perspective projection
void handleResize(int w, int h) {
	avdl_engine_resize(&engine, w, h);
}

#if DD_PLATFORM_NATIVE
struct dd_async_call dd_asyncCall = {0};
int dd_isAsyncCallActive = 0;
#endif

// constant update - this runs a specific number of times per second
void update() {

	#if DD_PLATFORM_NATIVE
	if (avdl_engine_isPaused(&engine)) {
		return;
	}
	#endif

	/*
	#if DD_PLATFORM_NATIVE
	// handle asynchronous calls
	if (dd_isAsyncCallActive) {
		pthread_mutex_lock(&asyncCallMutex);
		if (dd_asyncCall.isComplete) {
			dd_isAsyncCallActive = 0;
			if (dd_asyncCall.callback) {
				dd_asyncCall.callback(dd_asyncCall.context);
			}
		}
		pthread_mutex_unlock(&asyncCallMutex);
	}
	#endif
	*/

	avdl_engine_update(&engine);

	// prepare next frame
	#if DD_PLATFORM_NATIVE
	draw();
	#endif

	// close the game
	if (dd_flag_exit) {

/*
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
		dd_log("get method");
		jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "CloseApplication", "()V");
		dd_log("call method");
		(*(*jniEnv)->CallStaticVoidMethod)(jniEnv, clazz, MethodID);
		dd_log("detach");
		if (getEnvStat == JNI_EDETACHED) {
			(*jvm)->DetachCurrentThread(jvm);
		}
		*/
		#if DD_PLATFORM_ANDROID
		clean();
		exit(0);
		#endif
	}

}

// constant draw - called only when it needs to
void draw() {

	#if DD_PLATFORM_ANDROID
	if (dd_flag_exit) {
		return;
	}
	#endif

	avdl_engine_draw(&engine);
}

void handleKeyboardPress(unsigned char key, int x, int y) {

	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_lock(&updateDrawMutex);
	#endif

	input_key = key;

	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_unlock(&updateDrawMutex);
	#endif
}

void handleMousePress(int button, int state, int x, int y) {

	#if DD_PLATFORM_ANDROID
	avdl_input_AddInput(&avdl_input, button, state, x, y);
	#else
	int state_temp = 0;
	switch (state) {
		//case GLUT_DOWN:
		case 0:
			state_temp = DD_INPUT_MOUSE_TYPE_PRESSED;
			break;
		//case GLUT_UP:
		case 1:
			state_temp = DD_INPUT_MOUSE_TYPE_RELEASED;
			break;
	}

	int button_temp = 0;
	switch (button) {
		//case GLUT_LEFT_BUTTON:
		case 0:
			button_temp = DD_INPUT_MOUSE_BUTTON_LEFT;
			break;
		//case GLUT_MIDDLE_BUTTON:
		case 1:
			button_temp = DD_INPUT_MOUSE_BUTTON_MIDDLE;
			break;
		//case GLUT_RIGHT_BUTTON:
		case 2:
			button_temp = DD_INPUT_MOUSE_BUTTON_RIGHT;
			break;
	}
	avdl_input_AddInput(&avdl_input, button_temp, state_temp, x, y);
	#endif

}

void handlePassiveMotion(int x, int y) {
	avdl_input_AddPassiveMotion(&avdl_input, x, y);
}

#if DD_PLATFORM_ANDROID
void updateThread();
#endif

void onResume() {

	if (!avdl_state_initialised) return;

	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	dd_flag_exit = 0;
	dd_flag_focused = 1;
	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_unlock(&updateDrawMutex);
	#endif

	if (avdl_engine_isPaused(&engine)) {
		avdl_engine_setPaused(&engine, 0);
		#if DD_PLATFORM_ANDROID
		pthread_create(&updatePthread, NULL, &updateThread, NULL);
		#elif DD_PLATFORM_NATIVE
		// resume update with sdl?
		#endif
	}
}

void onPause() {

	/*
	if (nworld_constructor) {
		dd_log("cancel new world thread");
		pthread_cancel(newWorldPthread);
		nworld_constructor = 0;
		dd_log("canceled new world thread");
	}
	*/

	if (!avdl_state_initialised) return;

	if (!avdl_engine_isPaused(&engine)) {
		avdl_engine_setPaused(&engine, 1);
		#if DD_PLATFORM_ANDROID
		pthread_join(updatePthread, NULL);
		#endif
	}
	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	dd_flag_focused = 0;
	//dd_flag_initialised = 0;
	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_unlock(&updateDrawMutex);
	#endif
}

/*
 * Android specific functions that call the engine's events
 */
#if DD_PLATFORM_ANDROID

void updateThread() {
	while (!dd_flag_exit && !avdl_engine_isPaused(&engine)) {
		pthread_mutex_lock(&updateDrawMutex);
		update();
		pthread_mutex_unlock(&updateDrawMutex);
		usleep(33333);
	}
}

/*
 * nativeInit : Called when the app is first created
 */
void Java_org_darkdimension_avdl_AvdlRenderer_nativeInit(JNIEnv* env, jobject thiz, jobject mainActivity) {

	pthread_mutex_lock(&jniMutex);
	// Global variables to access Java virtual machine and environment
	(*env)->GetJavaVM(env, &jvm);
	(*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_4);
	jclass classLocal = (*(*jniEnv)->FindClass)(jniEnv, "org/darkdimension/avdl/AvdlActivity");
	clazz = (*jniEnv)->NewGlobalRef(jniEnv, classLocal);

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
void Java_org_darkdimension_avdl_AvdlActivity_nativeDone(JNIEnv*  env) {
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
void Java_org_darkdimension_avdl_AvdlRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
	dd_width = w;
	dd_height = h;

	handleResize(w, h);
}

/*
 * nativeRender : Called every so often to draw a frame
 */
void Java_org_darkdimension_avdl_AvdlRenderer_nativeRender(JNIEnv* env) {
	pthread_mutex_lock(&updateDrawMutex);
	draw();
	pthread_mutex_unlock(&updateDrawMutex);
}

/*
 * nativeMouseInput* : Called when a mouse/touch event happens, this is asynchronous
 * 	but will set a flag that the engine can pick up when ready
 */
void Java_org_darkdimension_avdl_AvdlGLSurfaceView_nativeMouseInputDown(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	handleMousePress(DD_INPUT_MOUSE_BUTTON_LEFT, DD_INPUT_MOUSE_TYPE_PRESSED, mouseX, mouseY);
}

void Java_org_darkdimension_avdl_AvdlGLSurfaceView_nativeMouseInputUp(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	handleMousePress(DD_INPUT_MOUSE_BUTTON_LEFT, DD_INPUT_MOUSE_TYPE_RELEASED, mouseX, mouseY);
}

void Java_org_darkdimension_avdl_AvdlGLSurfaceView_nativeMouseInputMove(JNIEnv*  env, jobject obj, jint mouseX, jint mouseY) {
	handleMousePress(DD_INPUT_MOUSE_BUTTON_LEFT, DD_INPUT_MOUSE_TYPE_MOVE, mouseX, mouseY);
}

void Java_org_darkdimension_avdl_AvdlGLSurfaceView_nativeTogglePauseResume(JNIEnv* env) {
}

void Java_org_darkdimension_avdl_AvdlActivity_nativePause(JNIEnv* env) {
	if (dd_flag_exit == 0) {
		onPause();
	}
}

void Java_org_darkdimension_avdl_AvdlActivity_nativeResume(JNIEnv* env) {
	if (dd_flag_exit == 0) {
		onResume();
	}
}

void Java_org_darkdimension_avdl_AvdlActivity_nativeKeyDown(JNIEnv*  env, jobject obj, jint key) {
	if (dd_flag_exit == 0) {
		pthread_mutex_lock(&updateDrawMutex);
		input_key = key;
		pthread_mutex_unlock(&updateDrawMutex);
	}
}
#endif
