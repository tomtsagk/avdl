#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <unistd.h>
#include <string.h>

// Threads
#include <pthread.h>

// world interface and starting world
#include "avdl_cengine.h"

pthread_t updatePthread;

/*
 * android includes
 */
#if DD_PLATFORM_ANDROID

	#include <jni.h>

/*
 * cglut includes
 */
#elif DD_PLATFORM_NATIVE

	// audio
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_mixer.h>

	// curl
	//#include <curl/curl.h>

	#include "dd_async_call.h"

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
int input_mouse;
int input_mouse_button;
int input_mouse_state;
int input_mouse_x;
int input_mouse_y;

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
GLuint risingProgram = 0;

#if DD_PLATFORM_NATIVE
// threads
pthread_mutex_t asyncCallMutex;
#endif

#if DD_PLATFORM_ANDROID
pthread_mutex_t jniMutex;
#endif
pthread_mutex_t updateDrawMutex;

int avdl_state_initialised = 0;
int avdl_state_active = 0;

int dd_main(int argc, char *argv[]) {

	avdl_assetManager_init();

	/*
	 * parse command line arguments
	 */
	for (int i = 0; i < argc; i++) {

		// custom save directory (for specific platforms)
		if (strcmp(argv[i], "--avdl-save-dir") == 0) {
			i++;
			if (i >= argc) {
				dd_log("please provide a save directory after \"--avdl-save-dir\"");
				exit(-1);
			}
			strcpy(avdl_data_saveDirectory, argv[i]);
		}

	}

	dd_clearcolor_r = 0;
	dd_clearcolor_g = 0;
	dd_clearcolor_b = 0;

	#if DD_PLATFORM_NATIVE
	srand(time(NULL));
	#endif
	input_key = 0;
	input_mouse = 0;

	#if DD_PLATFORM_ANDROID
	// initialise pthread mutex for jni
	if (pthread_mutex_init(&jniMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for jni init failed");
		return -1;
	}
	#endif

	if (pthread_mutex_init(&updateDrawMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for update/draw init failed");
		return -1;
	}

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
	// initialise glut and a window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//glutInitContextVersion(2, 0);
	//glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitWindowSize(dd_gameInitWindowWidth, dd_gameInitWindowHeight);
	glutCreateWindow(gameTitle);

	// init opengl
	glewInit();
	#endif

	avdl_engine_init_opengl();

	/*
	 * string3d initialisation for displaying text
	 */
	if (dd_string3d_isActive()) {
		dd_string3d_init();
	}

	#if DD_PLATFORM_NATIVE

	// audio is meant to be active
	if (dd_hasAudio) {

		/*
		 * initialise audio, if it fails, don't play audio at all
		 * during the game
		 */
		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			dd_log("avdl: error initialising audio");
			dd_hasAudio = 0;
		}
		else {
			if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
				dd_log("avdl: error initialising audio mixer");
				dd_hasAudio = 0;
			}
		}

	} // init audio

	// audio is off - display message about it
	if (!dd_hasAudio) {
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
	#if DD_PLATFORM_NATIVE
	// give functions to glut
	glutDisplayFunc(draw);
	glutReshapeFunc(handleResize);
	glutKeyboardFunc(handleKeyboardPress);
	glutMouseFunc(handleMousePress);
	glutPassiveMotionFunc(handlePassiveMotion);
	glutMotionFunc(handlePassiveMotion);

	onResume();

	// start the loop
	glutMainLoop();

	clean();
	#elif DD_PLATFORM_ANDROID
	onResume();
	#endif

	// everything ok
	return 0;
}

// clean leftovers
void clean() {
	if (!avdl_state_initialised) return;
	avdl_state_initialised = 0;

	if (cworld) {
		cworld->clean(cworld);
		cworld = 0;
	}

	#if DD_PLATFORM_ANDROID
	pthread_mutex_destroy(&jniMutex);
	#endif
	pthread_mutex_destroy(&updateDrawMutex);

	#if DD_PLATFORM_NATIVE
	Mix_Quit();
	SDL_Quit();

	//curl_global_cleanup();
	#endif
}

#if DD_PLATFORM_ANDROID
static void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat xmin, xmax, ymin, ymax;

	ymax = zNear * (GLfloat)tan(fovy * PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustumx((GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
		(GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
		(GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536));
}
#endif

void dd_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar) {

	float ymax, xmax;
	ymax = znear * tanf(fovyDegrees * M_PI / 360.0);
	xmax = ymax * aspectRatio;

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

	dd_fovaspect_set((double) w /(double) h);

	// perspective projection matrix
	dd_perspective((float *)&matPerspective, dd_fovy_get(), dd_fovaspect_get(), 1.0, 200.0);

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

	#if DD_PLATFORM_ANDROID
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
		if (nworld && nworld_ready && avdl_assetManager_isReady()) {

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
	#elif DD_PLATFORM_NATIVE
	// A new world has been flagged to be loaded
	if (nworld_constructor) {

		// world is ready to be viewed
		if (nworld_ready) {
			dd_world_change(nworld_size, nworld_constructor);
		}

		/*
		// The world has finished loading
		if (nworld && nworld_ready) {

			// Cancel async calls
			dd_isAsyncCallActive = 0;

			// free any previous world
			if (cworld) {
				cworld->clean(cworld);
				cworld = 0;
			}

			// Apply the new world
			cworld = nworld;
			nworld = 0;

			// From this point on, new world can be set
			nworld_constructor = 0;
			nworld_size = 0;
			nworld_ready = 0;
			loadingNewWorld = 0;

			// notify the world that it has loaded assets
			cworld->onload(cworld);

			// resize the new world
			if (cworld->resize) {
				cworld->resize(cworld);
			}

		}
		else
		// The new world has not started loading, so start loading it
		if (!loadingNewWorld) {
			loadingNewWorld = 1;
			newWorldData.size = nworld_size;
			newWorldData.constructor = nworld_constructor;
			pthread_t thread;
			pthread_create(&thread, NULL, newWorld_loading_thread_function, &newWorldData);
			pthread_detach(thread);

		}
		*/
	}
	#endif

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

	// handle input
	if (cworld && cworld->key_input && input_key) {
		cworld->key_input(cworld, input_key);
		input_key = 0;
	}

	if (cworld && cworld->mouse_input && input_mouse) {
		cworld->mouse_input(cworld, input_mouse_button, input_mouse_state);
		input_mouse = 0;
	}

	// update world
	if (cworld && cworld->update) {
		cworld->update(cworld);
	}

	// close the game
	if (dd_flag_exit) {
		//clean();

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
		exit(0);
	}

	// asset loader will load any new assets
	if (avdl_assetManager_hasAssets() && !avdl_assetManager_isLoading()) {
		avdl_assetManager_loadAssetsAsync();
	}

	// prepare next frame
	#if DD_PLATFORM_NATIVE
	glutPostRedisplay();
	//glutTimerFunc(33, update, 0);
	glutTimerFunc(25, update, 0);
	#endif
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
	glutSwapBuffers();
	#endif
}

void handleKeyboardPress(unsigned char key, int x, int y) {
	pthread_mutex_lock(&updateDrawMutex);
	input_key = key;
	pthread_mutex_unlock(&updateDrawMutex);
}

void handleMousePress(int button, int state, int x, int y) {

	#if DD_PLATFORM_ANDROID
	input_mouse_button = button;
	input_mouse_state = state;
	input_mouse_x = x;
	input_mouse_y = y;
	input_mouse = 1;
	#else
	input_mouse_x = x;
	input_mouse_y = y;
	switch (state) {
		case GLUT_DOWN:
			input_mouse_state = DD_INPUT_MOUSE_TYPE_PRESSED;
			break;
		case GLUT_UP:
			input_mouse_state = DD_INPUT_MOUSE_TYPE_RELEASED;
			break;
	}

	switch (button) {
		case GLUT_LEFT_BUTTON:
			input_mouse_button = DD_INPUT_MOUSE_BUTTON_LEFT;
			input_mouse = 1;
			break;
		case GLUT_MIDDLE_BUTTON:
			input_mouse_button = DD_INPUT_MOUSE_BUTTON_MIDDLE;
			input_mouse = 1;
			break;
		case GLUT_RIGHT_BUTTON:
			input_mouse_button = DD_INPUT_MOUSE_BUTTON_RIGHT;
			input_mouse = 1;
			break;
	}
	#endif

}

void handlePassiveMotion(int x, int y) {
	input_mouse_x = x;
	input_mouse_y = y;
}

#if DD_PLATFORM_ANDROID
void updateThread();
#endif

void onResume() {

	if (!avdl_state_initialised) return;

	pthread_mutex_lock(&updateDrawMutex);
	dd_flag_exit = 0;
	dd_flag_focused = 1;
	pthread_mutex_unlock(&updateDrawMutex);

	if (!avdl_state_active) {
		avdl_state_active = 1;
		#if DD_PLATFORM_ANDROID
		pthread_create(&updatePthread, NULL, &updateThread, NULL);
		#elif DD_PLATFORM_NATIVE
		glutTimerFunc(25, update, 0);
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
	pthread_mutex_lock(&updateDrawMutex);
	dd_flag_focused = 0;
	//dd_flag_initialised = 0;
	pthread_mutex_unlock(&updateDrawMutex);
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
