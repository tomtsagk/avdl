#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <unistd.h>

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

// Temporarily disabled for android
GLuint defaultProgram;
GLuint risingProgram;
GLuint testProgram;
#if DD_PLATFORM_NATIVE
extern GLuint fontProgram;

// threads
pthread_mutex_t asyncCallMutex;
#endif

pthread_mutex_t newWorldMutex;
pthread_mutex_t updateDrawMutex;
pthread_mutex_t updateThreadMutex;

int dd_main(int argc, char *argv[]) {

	dd_clearcolor_r = 0;
	dd_clearcolor_g = 0;
	dd_clearcolor_b = 0;

	#if DD_PLATFORM_NATIVE
	srand(time(NULL));
	#endif
	input_key = 0;
	input_mouse = 0;

	// initialise pthread mutex for new worlds
	if (pthread_mutex_init(&newWorldMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for new worlds init failed");
		return -1;
	}

	if (pthread_mutex_init(&updateDrawMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for update/draw init failed");
		return -1;
	}

	if (pthread_mutex_init(&updateThreadMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for update thread init failed");
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
	risingProgram = 0;
	glUseProgram(defaultProgram);

	/*
	 * strin3d initialisation for displaying text
	 */
	if (dd_string3d_isActive()) {
		dd_string3d_init();
	}

	#if DD_PLATFORM_NATIVE
	// init audio
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		dd_log("avdl: error initialising audio");
		exit(-1);
	}

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
		dd_log("avdl: error initialising audio mixer");
		dd_hasAudio = 0;
		//exit(-1);
	}
	#endif

	// initialise world
	nworld_size = 0;
	nworld_constructor = 0;
	dd_world_change(dd_default_world_size, dd_default_world_constructor);

	/* commented out - for now
	dd_log("Vendor graphic card: %s", glGetString(GL_VENDOR));
	dd_log("Renderer: %s", glGetString(GL_RENDERER));
	dd_log("Version GL: %s", glGetString(GL_VERSION));
	dd_log("Version GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	*/

	#if DD_PLATFORM_NATIVE
	// give functions to glut
	glutDisplayFunc(draw);
	glutReshapeFunc(handleResize);
	glutKeyboardFunc(handleKeyboardPress);
	glutMouseFunc(handleMousePress);
	glutPassiveMotionFunc(handlePassiveMotion);
	glutMotionFunc(handlePassiveMotion);

	//glutTimerFunc(33, update, 0);
	//glutTimerFunc(25, update, 0);

	dd_flag_initialised = 1;
	onResume();

	// start the loop
	glutMainLoop();

	clean();
	#elif DD_PLATFORM_ANDROID
	dd_flag_initialised = 1;
	onResume();
	#endif

	// everything ok
	return 0;
}

// clean leftovers
void clean() {
	dd_flag_initialised = 0;

	if (cworld) {
		cworld->clean(cworld);
		cworld = 0;
	}

	pthread_mutex_destroy(&newWorldMutex);
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

	if (cworld->resize) {
		cworld->resize(cworld);
	}
}

// data passed to the new world thread
struct newWorld_thread_data {
	int size;
	void (*constructor)(struct dd_world*);
};

void *newWorld_loading_thread_function(void *srcdata) {

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
	#endif

	// parse data
	struct newWorld_thread_data *data;
	data = srcdata;

	// initialise the new world locally
	struct dd_world *newWorld = 0;

	// allocate new world and construct it
	newWorld = malloc(data->size);
	data->constructor(newWorld);

	#if DD_PLATFORM_ANDROID
	if (getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#endif

	pthread_mutex_lock(&newWorldMutex);
	nworld = newWorld;
	pthread_mutex_unlock(&newWorldMutex);

	pthread_exit(NULL);
}

#if DD_PLATFORM_NATIVE
struct dd_async_call dd_asyncCall = {0};
int dd_isAsyncCallActive = 0;
#endif

// loading new world variables
struct newWorld_thread_data newWorldData;

// constant update - this runs a specific number of times per second
void update() {

	#if DD_PLATFORM_NATIVE
	if (!dd_flag_updateThread) {
		return;
	}
	#endif

	#if DD_PLATFORM_ANDROID
	// a new world is signaled to be loaded
	if (nworld_constructor) {

		/* `nworld` is the only variable that has to be
		 * accessed from one thread at a time
		 */
		pthread_mutex_lock(&newWorldMutex);

		// the new world has not started loading, so start loading it
		if (!nworld_loading) {

			// set flag that world is loading
			nworld_loading = 1;

			// prepare data to pass to the thread
			newWorldData.size = nworld_size;
			newWorldData.constructor = nworld_constructor;

			// start a thread to load new world
			pthread_t thread;
			pthread_create(&thread, NULL, newWorld_loading_thread_function, &newWorldData);
			pthread_detach(thread);
		}
		else
		// The world has finished loading
		if (nworld && nworld_ready) {

			/*
			// Cancel async calls
			dd_isAsyncCallActive = 0;
			*/

			// free any previous world
			if (cworld) {
				cworld->clean(cworld);
				cworld = 0;
			}

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

		// unlock mutex for `nworld`
		pthread_mutex_unlock(&newWorldMutex);
	}
	#elif DD_PLATFORM_NATIVE
	// A new world has been flagged to be loaded
	if (nworld_constructor) {

		// world is ready to be viewed
		if (nworld_ready) {
			dd_world_change(nworld_size, nworld_constructor);
		}

		/*
		pthread_mutex_lock(&newWorldMutex);
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
		pthread_mutex_unlock(&newWorldMutex);
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

	if (!dd_flag_initialised) return;

	pthread_mutex_lock(&updateDrawMutex);
	dd_flag_exit = 0;
	dd_flag_focused = 1;
	pthread_mutex_unlock(&updateDrawMutex);

	if (dd_flag_initialised && !dd_flag_updateThread) {
		dd_flag_updateThread = 1;
		#if DD_PLATFORM_ANDROID
		pthread_create(&updatePthread, NULL, &updateThread, NULL);
		#elif DD_PLATFORM_NATIVE
		glutTimerFunc(25, update, 0);
		#endif
	}
}

void onPause() {

	if (!dd_flag_initialised) return;

	if (dd_flag_updateThread) {
		dd_flag_updateThread = 0;
		#if DD_PLATFORM_ANDROID
		pthread_join(updatePthread, NULL);
		#endif
	}
	pthread_mutex_lock(&updateDrawMutex);
	dd_flag_focused = 0;
	dd_flag_initialised = 0;
	pthread_mutex_unlock(&updateDrawMutex);
}

/*
 * Android specific functions that call the engine's events
 */
#if DD_PLATFORM_ANDROID

void updateThread() {
	while (!dd_flag_exit && dd_flag_updateThread) {
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
void Java_org_darkdimension_avdl_AvdlRenderer_nativeInit(JNIEnv* env, jobject thiz) {

	// Global variables to access Java virtual machine and environment
	(*env)->GetJavaVM(env, &jvm);
	(*jvm)->GetEnv(jvm, &jniEnv, JNI_VERSION_1_4);
	jclass classLocal = (*(*jniEnv)->FindClass)(jniEnv, "org/darkdimension/avdl/AvdlActivity");
	clazz = (*jniEnv)->NewGlobalRef(jniEnv, classLocal);

	// Initialises the engine and the first world
	dd_main(0, 0);

}

/*
 * nativeDone : Called when closing the app
 */
void Java_org_darkdimension_avdl_AvdlActivity_nativeDone(JNIEnv*  env) {
	dd_flag_exit = 1;
	clean();
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
	onPause();
}

void Java_org_darkdimension_avdl_AvdlActivity_nativeResume(JNIEnv* env) {
	onResume();
}

void Java_org_darkdimension_avdl_AvdlActivity_nativeKeyDown(JNIEnv*  env, jobject obj, jint key) {
	pthread_mutex_lock(&updateDrawMutex);
	input_key = key;
	pthread_mutex_unlock(&updateDrawMutex);
}
#endif
