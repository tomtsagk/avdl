#include "avdl_engine.h"
#include "dd_game.h"

extern unsigned char input_key;
extern struct AvdlInput avdl_input;

extern int totalAssets;
extern int totalAssetsLoaded;

int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	//o->nworld_ready = 0;
	nworld_ready = 0;
	o->nworld_loading = 0;
	o->nworld_size = 0;
	o->nworld_constructor = 0;

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

int avdl_engine_resize(struct avdl_engine *o) {
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
