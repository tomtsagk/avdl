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

	extern SDL_Window* mainWindow;
	extern SDL_GLContext mainGLContext;
	extern SDL_TimerID timer;

#endif

/*
 * general event functions
 */
void update();
void draw();
void handleResize(int w, int h);
void clean();

void onResume();
void onPause();

int avdl_engine_init_opengl();

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

// shaders
GLuint defaultProgram;
GLuint fontProgram;
GLuint currentProgram;

#if DD_PLATFORM_NATIVE
// threads
//pthread_mutex_t asyncCallMutex;
#endif

#if DD_PLATFORM_ANDROID
pthread_mutex_t jniMutex;
#endif

int avdl_state_initialised = 0;
int avdl_state_active = 0;

struct avdl_achievements *achievements = 0;

int avdl_verify = 0;
int avdl_quiet = 0;

int dd_main(int argc, char *argv[]) {

	//freopen("error.log", "w", stdout);
	dd_log("about to parse arguments");
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
			avdl_verify = 1;
		}
		else
		// quiet mode
		if (strcmp(argv[i], "-q") == 0) {
			avdl_quiet = 1;
		}

	}

	#ifdef AVDL_STEAM
	if (!avdl_verify) {
		if (!avdl_steam_init()) {
			dd_log("avdl: error initialising steam");
			return -1;
		}
	}
	#endif

	dd_log("initialising avdl systems");
	achievements = avdl_achievements_create();
	avdl_initProjectLocation();
	avdl_assetManager_init();

	#if defined(_WIN32) || defined(WIN32)
	if (_wchdir(avdl_getProjectLocation()) != 0) {
		wprintf(L"avdl: failed to change directory: %lS", _wcserror(errno));
		return -1;
	}
	#endif

	dd_clearcolor_r = 0;
	dd_clearcolor_g = 0;
	dd_clearcolor_b = 0;

	#if DD_PLATFORM_NATIVE
	srand(time(NULL));
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

	// initialise pre-game data to defaults then to game-specifics
	dd_gameInitDefault();
	dd_gameInit();

	#if DD_PLATFORM_NATIVE

	if (!avdl_verify) {
		dd_log("initialising sdl window");
		// Initialise SDL window
		//int sdlError = SDL_Init(SDL_INIT_EVERYTHING);
		int sdlError = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		if (sdlError < 0) {
			dd_log("avdl: error initialising SDL2: %s", SDL_GetError());
			return -1;
		}
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
		int width = dd_gameInitWindowWidth;
		int height = dd_gameInitWindowHeight;
		mainWindow = SDL_CreateWindow(gameTitle, SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		if (mainWindow == NULL) {
			dd_log("avdl: failed to create SDL2 window: %s\n", SDL_GetError());
			return -1;
		}
		mainGLContext = SDL_GL_CreateContext(mainWindow);
		if (mainGLContext == NULL) {
			dd_log("avdl: failed to create OpenGL context: %s\n", SDL_GetError());
		}
		handleResize(dd_window_width(), dd_window_height());

		// init opengl
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			dd_log("avdl: glew failed to initialise: %s\n", glewGetErrorString(glewError));
			return -1;
		}
	}

	#endif

	if (!avdl_verify) {
		avdl_engine_init_opengl();

		/*
		 * string3d initialisation for displaying text
		 */
		if (dd_string3d_isActive()) {
			dd_string3d_init();
		}
	}

	#if DD_PLATFORM_NATIVE

	if (avdl_verify) {
		dd_hasAudio = 0;
	}

	// audio is meant to be active
	if (dd_hasAudio) {

		/*
		 * initialise audio, if it fails, don't play audio at all
		 * during the game
		 */
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
			dd_log("avdl: error initialising audio mixer");
			dd_hasAudio = 0;
		}
		// audio initialisation succeeded
		else {
			Mix_Init(MIX_INIT_OPUS | MIX_INIT_OGG);

			// make sure there's at least 8 channels
			dd_numberOfAudioChannels = Mix_AllocateChannels(-1);
			if (dd_numberOfAudioChannels < 8) {
				dd_numberOfAudioChannels = Mix_AllocateChannels(8);
			}

			// start at full volume
			avdl_music_setVolume(100);
			avdl_sound_setVolume(100);

		}

	} // init audio

	// audio is off - display message about it
	if (!dd_hasAudio && !avdl_verify) {
		dd_log("Game will play without audio");
	}

	#endif

	// initialise world
	cworld = 0;
	nworld_size = dd_default_world_size;
	nworld_constructor = dd_default_world_constructor;
	nworld_ready = 1;

	/* commented out - for now
	dd_log("Vendor graphic card: %s", glGetString(GL_VENDOR));
	dd_log("Renderer: %s", glGetString(GL_RENDERER));
	dd_log("Version GL: %s", glGetString(GL_VERSION));
	dd_log("Version GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	*/

	avdl_state_initialised = 1;
	onResume();
	#if DD_PLATFORM_NATIVE

	if (!avdl_verify) {
		// start the loop
		int isRunning = 1;
		SDL_Event event;
		//while (isRunning && SDL_WaitEvent(&event) && !dd_flag_exit) {
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

	if (avdl_verify) {

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
	if (!avdl_quiet) {
		dd_log("avdl: cleaning data");
	}
	*/
	if (!avdl_state_initialised) return;
	avdl_state_initialised = 0;

	#ifdef AVDL_STEAM
	if (!avdl_verify) {
		avdl_steam_shutdown();
	}
	#endif
	avdl_achievements_clean(achievements);
	avdl_cleanProjectLocation();

	if (cworld) {
		cworld->clean(cworld);
		cworld = 0;
	}

	#if DD_PLATFORM_ANDROID
	pthread_mutex_destroy(&jniMutex);
	#endif

	#if defined(_WIN32) || defined(WIN32)
	#else
	pthread_mutex_destroy(&updateDrawMutex);
	#endif

	#if DD_PLATFORM_NATIVE
	if (!avdl_verify) {
		// destroy window
		SDL_GL_DeleteContext(mainGLContext);
		SDL_DestroyWindow(mainWindow);

		Mix_Quit();
		SDL_Quit();
	}

	//curl_global_cleanup();
	#endif
	/*
	if (!avdl_quiet) {
		dd_log("avdl: done cleaning data");
	}
	*/
}

void dd_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority) {

	float ymax, xmax;
	if (ypriority) {
		ymax = znear * tanf(fovyDegrees * M_PI / 360.0);
		xmax = ymax * aspectRatio;
	}
	else {
		xmax = znear * tanf(fovyDegrees * M_PI / 360.0);
		ymax = xmax * aspectRatio;
	}

	float left = -xmax;
	float right = xmax;
	float bottom = -ymax;
	float top = ymax;

	float temp, temp2, temp3, temp4;
	temp = 2.0 * znear;
	temp2 = right - left;
	temp3 = top - bottom;
	temp4 = zfar - znear;
	matrix[0] = temp / temp2;
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.0;
	matrix[4] = 0.0;
	matrix[5] = temp / temp3;
	matrix[6] = 0.0;
	matrix[7] = 0.0;
	matrix[8] = (right + left) / temp2;
	matrix[9] = (top + bottom) / temp3;
	matrix[10] = (-zfar - znear) / temp4;
	matrix[11] = -1.0;
	matrix[12] = 0.0;
	matrix[13] = 0.0;
	matrix[14] = (-temp * zfar) / temp4;
	matrix[15] = 0.0;
}

struct dd_matrix matPerspective;

// handle resize with perspective projection
void handleResize(int w, int h) {
	glViewport(0, 0, w, h);

	int ypriority;
	if (w > h) {
		dd_fovaspect_set((double) w /(double) h);
		ypriority = 1;
	}
	else {
		dd_fovaspect_set((double) h /(double) w);
		ypriority = 0;
	}

	// perspective projection matrix
	dd_perspective((float *)&matPerspective, dd_fovy_get(), dd_fovaspect_get(), 1.0, 200.0, ypriority);

	if (cworld && cworld->resize) {
		cworld->resize(cworld);
	}
}

#if DD_PLATFORM_NATIVE
struct dd_async_call dd_asyncCall = {0};
int dd_isAsyncCallActive = 0;
#endif

// constant update - this runs a specific number of times per second
void update() {

	#if DD_PLATFORM_NATIVE
	if (!avdl_state_active) {
		return;
	}
	#endif

	#ifdef AVDL_STEAM
	avdl_steam_update();
	#endif

	// a new world is signaled to be loaded
	if (nworld_constructor) {

		// the new world has not started loading, so start loading it
		if (!nworld_loading) {

			// set flag that world is loading
			nworld_loading = 1;

			// clear everything loading on asset manager
			avdl_assetManager_clear();

			// allocate new world and construct it
			nworld = malloc(nworld_size);
			nworld_constructor(nworld);

			// from now on, loading new assets is not allowed
			avdl_assetManager_lockLoading();

		}
		else
		// The world has finished loading
		if (nworld && nworld_ready && avdl_assetManager_getLoadedProportion() >= 1.0) {

			/*
			// Cancel async calls
			dd_isAsyncCallActive = 0;
			*/

			// free any previous world
			if (cworld) {
				cworld->clean(cworld);
				cworld = 0;
			}

			// from now on, new assets can be loaded again
			avdl_assetManager_unlockLoading();

			// Apply the new world
			cworld = nworld;
			nworld = 0;

			// from this point on, new world can be set
			nworld_constructor = 0;
			nworld_size = 0;
			nworld_ready = 0;
			nworld_loading = 0;

			// notify the world that it has loaded assets
			cworld->onload(cworld);

			// resize the new world
			if (cworld->resize) {
				cworld->resize(cworld);
			}

		}

	}

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

	// handle key input
	if (cworld && cworld->key_input && input_key) {
		cworld->key_input(cworld, input_key);
		input_key = 0;
	}

	// handle mouse input
	if (cworld && cworld->mouse_input && avdl_input_GetInputTotal(&avdl_input) > 0) {
		int totalInput = avdl_input_GetInputTotal(&avdl_input);
		for (int i = 0; i < totalInput; i++) {
			cworld->mouse_input(cworld, avdl_input_GetButton(&avdl_input, i), avdl_input_GetState(&avdl_input, i));
		}
		avdl_input_ClearInput(&avdl_input);
	}

	// update world
	if (cworld && cworld->update) {
		cworld->update(cworld);
	}

	// asset loader will load any new assets
	if (avdl_assetManager_hasAssetsToLoad() && !avdl_assetManager_isLoading()) {
		avdl_assetManager_loadAll();
	}

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

	// clear everything
	glClearColor(dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// reset view
	dd_matrix_globalInit();

	// draw world
	if (cworld && cworld->draw) {
		cworld->draw(cworld);
	}

	#if DD_PLATFORM_NATIVE
	// show result
	SDL_GL_SwapWindow(mainWindow);
	#endif
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

	if (!avdl_state_active) {
		avdl_state_active = 1;
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

	if (avdl_state_active) {
		avdl_state_active = 0;
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

int avdl_engine_init_opengl() {

	avdl_opengl_generateContextId();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.8, 0.6, 1.0, 1);

	/*
	 * load shaders
	 */
	defaultProgram = avdl_loadProgram(avdl_shaderDefault_vertex, avdl_shaderDefault_fragment);
	if (!defaultProgram) {
		dd_log("avdl: error loading shaders");
		exit(-1);
	}

	fontProgram = avdl_loadProgram(avdl_shaderFont_vertex, avdl_shaderFont_fragment);
	if (!fontProgram) {
		dd_log("avdl: error loading font shaders");
		exit(-1);
	}

	glUseProgram(defaultProgram);
	currentProgram = defaultProgram;

	return 0;

}

/*
 * Android specific functions that call the engine's events
 */
#if DD_PLATFORM_ANDROID

void updateThread() {
	while (!dd_flag_exit && avdl_state_active) {
		pthread_mutex_lock(&updateDrawMutex);
		update();
		pthread_mutex_unlock(&updateDrawMutex);
		//usleep(33333);
		usleep(25000);
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


	if (!avdl_state_initialised) {
		// Initialises the engine and the first world
		dd_main(0, 0);
	}
	else {
		avdl_engine_init_opengl();
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
	/*
	dd_flag_exit = 1;
	clean();
	*/
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
