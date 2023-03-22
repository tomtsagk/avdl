#ifndef AVDL_ENGINE_H
#define AVDL_ENGINE_H

#include "avdl_graphics.h"

struct avdl_engine {
	#if DD_PLATFORM_NATIVE
	SDL_Window *window;
	SDL_GLContext glContext;
	#endif

	int isRunning;
	int isPaused;
};

int avdl_engine_init(struct avdl_engine *o);
int avdl_engine_clean(struct avdl_engine *o);
int avdl_engine_draw(struct avdl_engine *o);
int avdl_engine_loop(struct avdl_engine *o);

void avdl_engine_setPaused(struct avdl_engine *o, int state);
int avdl_engine_isPaused(struct avdl_engine *o);

#endif
