#include "dd_game.h"
#include "dd_math.h"
#include "dd_fov.h"

int dd_flag_initialised = 0;
int dd_flag_focused = 0;
int dd_flag_updateThread = 0;
int dd_flag_exit = 0;

#if DD_PLATFORM_ANDROID
int dd_width = 0;
int dd_height = 0;
#else
#include <SDL2/SDL.h>

SDL_Window* mainWindow;
SDL_GLContext mainGLContext;
SDL_TimerID timer;
#endif

float dd_clearcolor_r;
float dd_clearcolor_g;
float dd_clearcolor_b;

// initialise game init variables
char *gameTitle = 0;
int dd_gameInitWindowWidth = 0;
int dd_gameInitWindowHeight = 0;

void dd_gameInitDefault() {
	gameTitle = "game";
	dd_gameInitWindowWidth = 640;
	dd_gameInitWindowHeight = 480;
}

#if DD_PLATFORM_NATIVE
int dd_window_width() {
	int w, h;
	SDL_GetWindowSize(mainWindow, &w, &h);
	return w;
}

int dd_window_height() {
	int w, h;
	SDL_GetWindowSize(mainWindow, &w, &h);
	return h;
}
#endif

// screen limits
float dd_screen_width_get (float z) {
	return dd_screen_height_get(z) *dd_fovaspect_get();
}

float dd_screen_height_get(float z) {
	return dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) ) *z *2;
}

float dd_screen_distance_getw(float width) {
	return dd_screen_distance_geth(width /dd_fovaspect_get());
}

float dd_screen_distance_geth(float height) {
	return (height/2) /dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) );
}
