#include "avdl_engine.h"
#include "dd_game.h"

#include "dd_matrix.h"
#include "dd_fov.h"
#include "avdl_graphics.h"
#include "avdl_cengine.h"

#include <math.h>

extern int totalAssets;
extern int totalAssetsLoaded;

static void avdl_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority);

extern int dd_flag_exit;

#ifndef AVDL_DIRECT3D11
int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	//o->nworld_ready = 0;
	nworld_ready = 0;
	o->nworld_loading = 0;
	o->nworld_size = 0;
	o->nworld_constructor = 0;

	o->input_key = 0;

	#ifdef AVDL_STEAM
	if (!o->verify) {
		if (!avdl_steam_init()) {
			dd_log("avdl: error initialising steam");
			return -1;
		}
	}
	#endif

	avdl_input_Init(&o->input);

	#if defined(_WIN32) || defined(WIN32)
	const PROJ_LOC_TYPE *proj_loc = avdl_getProjectLocation();
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

	o->achievements = avdl_achievements_create();
	avdl_assetManager_init();

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

	// audio
	#if DD_PLATFORM_NATIVE

	if (o->verify) {
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
	if (!dd_hasAudio && !o->verify) {
		dd_log("avdl error: Game will play without audio");
	}

	#endif

	// window
	#if DD_PLATFORM_NATIVE

	srand(time(NULL));

	// Initialise SDL window
	int sdlError = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (sdlError < 0) {
		dd_log("avdl: error initialising SDL2: %s", SDL_GetError());
		return -1;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	int width = dd_gameInitWindowWidth;
	int height = dd_gameInitWindowHeight;
	o->window = SDL_CreateWindow(gameTitle, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (o->window == NULL) {
		dd_log("avdl: failed to create SDL2 window: %s\n", SDL_GetError());
		return -1;
	}
	o->glContext = SDL_GL_CreateContext(o->window);
	if (o->glContext == NULL) {
		dd_log("avdl: failed to create OpenGL context: %s\n", SDL_GetError());
	}
	//handleResize(dd_window_width(), dd_window_height());
	#endif

	#if DD_PLATFORM_NATIVE
	avdl_engine_resize(o, dd_window_width(), dd_window_height());
	#endif

	avdl_graphics_Init();

	/*
	 * string3d initialisation for displaying text
	 */
	if (dd_string3d_isActive()) {
		dd_string3d_init();
	}

	return 0;
}
#endif

int avdl_engine_clean(struct avdl_engine *o) {

	#ifdef AVDL_DIRECT3D11
	#else
	avdl_achievements_clean(o->achievements);

	if (o->cworld) {
		o->cworld->clean(o->cworld);
		o->cworld = 0;
	}

	#if DD_PLATFORM_NATIVE
	// destroy window
	SDL_GL_DeleteContext(o->glContext);
	SDL_DestroyWindow(o->window);

	Mix_Quit();
	SDL_Quit();
	#endif

	#endif

	#ifdef AVDL_STEAM
	if (!o->verify) {
		avdl_steam_shutdown();
	}
	#endif
	return 0;
}

#ifndef AVDL_DIRECT3D11
int avdl_engine_draw(struct avdl_engine *o) {

	#if DD_PLATFORM_ANDROID
	if (dd_flag_exit) {
		return 0;
	}
	#endif

	// clear everything
	avdl_graphics_ClearToColour();
	avdl_graphics_ClearColourAndDepth();

	// reset view
	dd_matrix_globalInit();

	// draw world
	if (o->cworld && o->cworld->draw) {
		o->cworld->draw(o->cworld);
	}

	#if DD_PLATFORM_NATIVE
	// show result
	SDL_GL_SwapWindow(o->window);
	#endif

	return 0;
}
#endif

int avdl_engine_isPaused(struct avdl_engine *o) {
	return o->isPaused;
}

void avdl_engine_setPaused(struct avdl_engine *o, int state) {
	o->isPaused = state;
}

int avdl_engine_initWorld(struct avdl_engine *o, void (*constructor)(struct dd_world*), int size) {
	nworld_constructor = constructor;
	nworld_size = size;
	//o->nworld_constructor = constructor;
	//o->nworld_size = size;
	//o->nworld_ready = 1;
	nworld_ready = 1;
	return 0;
}

struct dd_matrix matPerspective;

int avdl_engine_resize(struct avdl_engine *o, int w, int h) {

	#ifdef AVDL_DIRECT3D11
	#else
	avdl_graphics_Viewport(0, 0, w, h);

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
	avdl_perspective((float *)&matPerspective, dd_fovy_get(), dd_fovaspect_get(), 1.0, 200.0, ypriority);

	if (o->cworld && o->cworld->resize) {
		o->cworld->resize(o->cworld);
	}
	#endif
	return 0;
}

int avdl_engine_update(struct avdl_engine *o) {

	#if DD_PLATFORM_NATIVE
	if (avdl_engine_isPaused(o)) {
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


	#ifdef AVDL_STEAM
	avdl_steam_update();
	#endif

	// a new world is signaled to be loaded
	if (nworld_constructor) {
	//if (o->nworld_constructor)

		int hasAssetManagerLoaded = 0;
		if (totalAssets == 0) {
			hasAssetManagerLoaded = 1;
		}
		else {
			if (totalAssetsLoaded == totalAssets) {
				hasAssetManagerLoaded = 1;
			}
		}

		// temp
		o->nworld_constructor = nworld_constructor;
		o->nworld_size = nworld_size;

		// the new world has not started loading, so start loading it
		if (!o->nworld_loading) {

			// set flag that world is loading
			o->nworld_loading = 1;

			// clear everything loading on asset manager
			avdl_assetManager_clear();

			// allocate new world and construct it
			o->nworld = malloc(o->nworld_size);
			o->nworld_constructor(o->nworld);

			// from now on, loading new assets is not allowed
			avdl_assetManager_lockLoading();

		}
		else
		// The world has finished loading
		//if (o->nworld && o->nworld_ready && avdl_assetManager_getLoadedProportion() >= 1.0)
		if (o->nworld && nworld_ready && hasAssetManagerLoaded) {

			/*
			// Cancel async calls
			dd_isAsyncCallActive = 0;
			*/

			// free any previous world
			if (o->cworld) {
				o->cworld->clean(o->cworld);
				o->cworld = 0;
			}

			// from now on, new assets can be loaded again
			avdl_assetManager_unlockLoading();

			// Apply the new world
			o->cworld = o->nworld;
			o->nworld = 0;

			// from this point on, new world can be set
			o->nworld_constructor = 0;
			nworld_constructor = 0;
			o->nworld_size = 0;
			//o->nworld_ready = 0;
			nworld_ready = 0;
			o->nworld_loading = 0;

			// notify the world that it has loaded assets
			o->cworld->onload(o->cworld);

			// resize the new world
			if (o->cworld->resize) {
				o->cworld->resize(o->cworld);
			}

		}

	}

	// handle key input
	if (o->cworld && o->cworld->key_input && o->input_key) {
		o->cworld->key_input(o->cworld, o->input_key);
		o->input_key = 0;
	}

	// handle mouse input
	if (o->cworld && o->cworld->mouse_input && avdl_input_GetInputTotal(&o->input) > 0) {
		int totalInput = avdl_input_GetInputTotal(&o->input);
		for (int i = 0; i < totalInput; i++) {
			o->cworld->mouse_input(o->cworld, avdl_input_GetButton(&o->input, i), avdl_input_GetState(&o->input, i));
		}
		avdl_input_ClearInput(&o->input);
	}

	// update world
	if (o->cworld && o->cworld->update) {
		o->cworld->update(o->cworld);
	}

	// asset loader will load any new assets
	if (avdl_assetManager_hasAssetsToLoad() && !avdl_assetManager_isLoading()) {
		avdl_assetManager_loadAll();
	}

	return 0;
}

void avdl_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority) {
	#ifndef AVDL_DIRECT3D11

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

	#endif
}

#ifndef AVDL_DIRECT3D11
static void handleMousePress(struct avdl_engine *o, int button, int state, int x, int y) {

	// SDL to AVDL conversion
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
	avdl_input_AddInput(&o->input, button_temp, state_temp, x, y);

}

int avdl_engine_loop(struct avdl_engine *o) {
	#if DD_PLATFORM_NATIVE

	if (o->verify) {
		avdl_engine_verify(o);
		return 0;
	}

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
					avdl_engine_resize(o, event.window.data1, event.window.data2);
				}
				break;
			case SDL_MOUSEMOTION:
				avdl_input_AddPassiveMotion(&o->input, event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				handleMousePress(o, 0, 0, event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONUP:
				handleMousePress(o, 0, 1, event.motion.x, event.motion.y);
				break;
			case SDL_KEYDOWN:
				// temporary keyboard controls
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					o->input_key = 27;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_A) {
					o->input_key = 97;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_D) {
					o->input_key = 100;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_W) {
					o->input_key = 119;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_S) {
					o->input_key = 115;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					o->input_key = 32;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					o->input_key = 13;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					o->input_key = 1;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
					o->input_key = 2;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					o->input_key = 3;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					o->input_key = 4;
				}
				break;
			}
		}

		//update();
		avdl_engine_update(o);

		// prepare next frame
		#if DD_PLATFORM_NATIVE
		avdl_engine_draw(o);
		//draw();
		#endif

		SDL_Delay(33.333);
	}
	#endif
	return 0;
}
#endif

void avdl_engine_verify(struct avdl_engine *o) {
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
