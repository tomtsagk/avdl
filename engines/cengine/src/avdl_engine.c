#include "avdl_engine.h"
#include "dd_game.h"

int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;

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
	#if DD_PLATFORM_NATIVE
	// destroy window
	SDL_GL_DeleteContext(o->glContext);
	SDL_DestroyWindow(o->window);

	Mix_Quit();
	SDL_Quit();
	#endif
}

int avdl_engine_draw(struct avdl_engine *o) {
	#if DD_PLATFORM_NATIVE
	// show result
	SDL_GL_SwapWindow(o->window);
	#endif
}

int avdl_engine_loop(struct avdl_engine *o) {

	/*
	// start the loop
	o->isRunning = 1;
	SDL_Event event;
	while (o->isRunning) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					o->isRunning = 0;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						//handleResize(event.window.data1, event.window.data2);
					}
					break;
				case SDL_MOUSEMOTION:
					//handlePassiveMotion(event.motion.x, event.motion.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
					//handleMousePress(0, 0, event.motion.x, event.motion.y);
					break;
				case SDL_MOUSEBUTTONUP:
					//handleMousePress(0, 1, event.motion.x, event.motion.y);
					break;
				case SDL_KEYDOWN:
					// temporary keyboard controls
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						//handleKeyboardPress(27, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_A) {
						//handleKeyboardPress(97, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_D) {
						//handleKeyboardPress(100, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						//handleKeyboardPress(119, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_S) {
						//handleKeyboardPress(115, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						//handleKeyboardPress(32, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						//handleKeyboardPress(13, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						//handleKeyboardPress(1, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
						//handleKeyboardPress(2, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						//handleKeyboardPress(3, 0, 0);
					}
					else
					if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
						//handleKeyboardPress(4, 0, 0);
					}
					break;
			}
		}
		//update();
		SDL_Delay(33.333);
	}
	*/
}

int avdl_engine_isPaused(struct avdl_engine *o) {
	return o->isPaused;
}

void avdl_engine_setPaused(struct avdl_engine *o, int state) {
	o->isPaused = state;
}
