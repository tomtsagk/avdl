#include "dd_game.h"
#include "dd_math.h"
#include "dd_fov.h"
#include "whereami.h"
#include <stdio.h>

int dd_flag_initialised = 0;
int dd_flag_focused = 0;
int dd_flag_updateThread = 0;
int dd_flag_exit = 0;

#if DD_PLATFORM_ANDROID
int dd_width = 0;
int dd_height = 0;
#else
#include "dd_opengl.h"

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

void dd_fullscreenToggle() {
	#if DD_PLATFORM_NATIVE
	Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	int isFullscreen = SDL_GetWindowFlags(mainWindow) & FullscreenFlag;
	SDL_SetWindowFullscreen(mainWindow, isFullscreen ? 0 : FullscreenFlag);
	#elif DD_PLATFORM_ANDROID
	#endif
}

char *dynamicProjectLocation = 0;
extern char *installLocation = "";
extern int installLocationDynamic;

const char *avdl_getProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	if (!dynamicProjectLocation) {
		return "";
	}
	else {
		return dynamicProjectLocation;
	}
	#else
	return PKG_LOCATION;
	#endif
}

void avdl_initProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	int length = wai_getExecutablePath(0, 0, 0);
	dynamicProjectLocation = malloc(sizeof(char) *(length+1));
	if (!dynamicProjectLocation) {
		printf("avdl error: unable to allocate memory for dynamic package location\n");
	}
	wai_getExecutablePath(dynamicProjectLocation, length, 0);
	dynamicProjectLocation[length] = '\0';

	int directoriesToSkip = 1;
	for (int i = length-1; i >= 0; i--) {
		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		if (dynamicProjectLocation[i] == '\\') {
		#else
		if (dynamicProjectLocation[i] == '/') {
		#endif
			if (directoriesToSkip > 0) {
				directoriesToSkip--;
			}
			else {
				dynamicProjectLocation[i+1] = '\0';
				directoriesToSkip = -1;
				break;
			}
		}
	}

	if (directoriesToSkip != -1) {
		printf("avdl error: unable to parse dynamic package location: %s\n", dynamicProjectLocation);
		free(dynamicProjectLocation);
		dynamicProjectLocation = 0;
		return;
	}
	#endif
}

void avdl_cleanProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	if (dynamicProjectLocation) {
		free(dynamicProjectLocation);
	}
	#endif
}
