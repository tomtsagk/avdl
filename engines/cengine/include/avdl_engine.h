#ifndef AVDL_ENGINE_H
#define AVDL_ENGINE_H

#include "avdl_graphics.h"
#include "dd_world.h"
#include "avdl_input.h"
#include "avdl_achievements.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_engine {

	#ifdef AVDL_DIRECT3D11
	/*
	HINSTANCE hInstance;
	int nCmdShow;
	HWND hwnd;
	*/
	#else
	#if DD_PLATFORM_NATIVE
	SDL_Window *window;
	SDL_GLContext glContext;
	#endif

	#endif

	int isRunning;
	int isPaused;

	struct dd_world *cworld;
	struct dd_world *nworld;
	int nworld_ready;
	int nworld_loading;
	void (*nworld_constructor)(struct dd_world*);
	int nworld_size;

	// settings
	int verify;
	int quiet;

	struct avdl_achievements *achievements;

	unsigned char input_key;

	struct AvdlInput input;
};

int avdl_engine_init(struct avdl_engine *o);
int avdl_engine_initWorld(struct avdl_engine *o, void (*constructor)(struct dd_world*), int size);
int avdl_engine_clean(struct avdl_engine *o);
int avdl_engine_draw(struct avdl_engine *o);

int avdl_engine_resize(struct avdl_engine *o, int w, int h);
int avdl_engine_update(struct avdl_engine *o);
int avdl_engine_loop(struct avdl_engine *o);

void avdl_engine_setPaused(struct avdl_engine *o, int state);
int avdl_engine_isPaused(struct avdl_engine *o);

void avdl_engine_verify(struct avdl_engine *o);

#ifdef __cplusplus
}
#endif

#endif
