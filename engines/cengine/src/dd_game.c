#include "dd_game.h"
#include "dd_math.h"
#include "dd_fov.h"
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

#if defined(_WIN32) || defined(WIN32)
wchar_t dynamicProjectLocationW[1000];

#include <windows.h>
#endif

#if defined(_WIN32) || defined(WIN32)
const wchar_t *avdl_getProjectLocation() {
#else
const char *avdl_getProjectLocation() {
#endif
	#ifdef AVDL_DYNAMIC_PKG_LOCATION

	#if defined(_WIN32) || defined(WIN32)
	if (!dynamicProjectLocationW) {
		return L"";
	}
	else {
		return dynamicProjectLocationW;
	}
	#else
	return "";
	#endif

	#else

	#if defined(_WIN32) || defined(WIN32)
	return LPKG_LOCATION;
	#else
	return PKG_LOCATION;
	#endif

	#endif
}

void avdl_initProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	//wprintf(L"get project loc\n");

	#if defined(_WIN32) || defined(WIN32)
	wchar_t *pointer = dynamicProjectLocationW;
	wchar_t *secondToLastSlash = 0;
	wchar_t *lastSlash = 0;
	int slashesLeft = 2;
	GetModuleFileNameW(NULL, dynamicProjectLocationW, 999);
	dynamicProjectLocationW[999] = L'\0';

	while (pointer[0] != L'\0') {
		if (pointer[0] == L'\\') {
			secondToLastSlash = lastSlash;
			lastSlash = pointer;
		}
		pointer++;
	}
	if (secondToLastSlash) {
		secondToLastSlash++;
		secondToLastSlash[0] = L'\0';
	}
	//wprintf(L"project loc: %lS\n", dynamicProjectLocationW);
	#else
	// not supported on non-windows os for now
	#endif

	#endif
}

void avdl_cleanProjectLocation() {
}
