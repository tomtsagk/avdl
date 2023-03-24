#include "avdl_engine.h"
#include "dd_game.h"

#include "dd_matrix.h"
#include "dd_fov.h"
#include "avdl_graphics.h"
#include "avdl_cengine.h"

#include <math.h>

extern unsigned char input_key;
extern struct AvdlInput avdl_input;

extern int totalAssets;
extern int totalAssetsLoaded;

static void avdl_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority);

int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	//o->nworld_ready = 0;
	nworld_ready = 0;
	o->nworld_loading = 0;
	o->nworld_size = 0;
	o->nworld_constructor = 0;

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

int avdl_engine_clean(struct avdl_engine *o) {

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
	return 0;
}

int avdl_engine_draw(struct avdl_engine *o) {

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
	return 0;
}

int avdl_engine_update(struct avdl_engine *o) {

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
	if (o->cworld && o->cworld->key_input && input_key) {
		o->cworld->key_input(o->cworld, input_key);
		input_key = 0;
	}

	// handle mouse input
	if (o->cworld && o->cworld->mouse_input && avdl_input_GetInputTotal(&avdl_input) > 0) {
		int totalInput = avdl_input_GetInputTotal(&avdl_input);
		for (int i = 0; i < totalInput; i++) {
			o->cworld->mouse_input(o->cworld, avdl_input_GetButton(&avdl_input, i), avdl_input_GetState(&avdl_input, i));
		}
		avdl_input_ClearInput(&avdl_input);
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
