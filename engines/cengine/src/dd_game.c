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
	if (dd_window_width() > dd_window_height()) {
		return dd_screen_height_get(z) *dd_fovaspect_get();
	}
	else {
		return dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) ) *z *2;
	}
}

float dd_screen_height_get(float z) {
	if (dd_window_width() > dd_window_height()) {
		return dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) ) *z *2;
	}
	else {
		return dd_screen_width_get(z) *dd_fovaspect_get();
	}
}

float dd_screen_distance_getw(float width) {
	if (dd_window_width() > dd_window_height()) {
		return dd_screen_distance_geth(width /dd_fovaspect_get());
	}
	else {
		return (width/2) /dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) );
	}
}

float dd_screen_distance_geth(float height) {
	if (dd_window_width() > dd_window_height()) {
		return (height/2) /dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) );
	}
	else {
		return dd_screen_distance_getw(height /dd_fovaspect_get());
	}
}

void dd_fullscreenToggle() {
	#if DD_PLATFORM_NATIVE
	Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	int isFullscreen = SDL_GetWindowFlags(mainWindow) & FullscreenFlag;
	SDL_SetWindowFullscreen(mainWindow, isFullscreen ? 0 : FullscreenFlag);
	#elif DD_PLATFORM_ANDROID
	#endif
}

int dd_canFullscreenToggle() {
#if DD_PLATFORM_ANDROID
	return 0;
#endif
	return 1;
}

#if defined(_WIN32) || defined(WIN32)
wchar_t dynamicProjectLocationW[1000];

#include <windows.h>
#endif

/*
 * By default build is in a dynamic location,
 * unless a package location was given
 */
#ifdef GAME_PKG_LOCATION
int game_pkg_location_type = GAME_PKG_LOCATION_TYPE_FIXED;
char *game_pkg_location_path = GAME_PKG_LOCATION;
#else
int game_pkg_location_type = GAME_PKG_LOCATION_TYPE_DYNAMIC;
char *game_pkg_location_path = 0;
#endif

char tempProjLoc[1024];

#if defined(_WIN32) || defined(WIN32)
wchar_t tempProjLocW[1024];
#endif

const PROJ_LOC_TYPE *avdl_getProjectLocation() {

	dd_log("about to get project location");
	if (game_pkg_location_type == GAME_PKG_LOCATION_TYPE_FIXED) {
		dd_log("fixed location");
		return game_pkg_location_path;
	}
	else
	if (game_pkg_location_type == GAME_PKG_LOCATION_TYPE_DYNAMIC) {
		dd_log("dynamic location");


		#if defined(_WIN32) || defined(WIN32)
		wchar_t *pointer = tempProjLocW;
		wchar_t *secondToLastSlash = 0;
		wchar_t *lastSlash = 0;
		int slashesLeft = 2;
		GetModuleFileNameW(NULL, tempProjLocW, 999);
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
		dd_log("final location: %lS", tempProjLocW);
		return tempProjLocW;
		#else

		// get path of binary
		int length = wai_getExecutablePath(NULL, 0, NULL);
		if (length < 400) {
			wai_getExecutablePath(tempProjLoc, length, 0);
		}
		else {
			dd_log("too long project path\n");
			return 0;
		}
		dd_log("location: %s", tempProjLoc);

		char slash;
		#if defined(_WIN32) || defined(WIN32)
		slash = '\\';
		#else
		slash = '/';
		#endif

		// lose last two files (so `/directory/bin/avdl` becomes `/directory/`)
		char *p = tempProjLoc +length -1;
		char *lastSlash = 0;
		char *secondToLastSlash = 0;
		while (p >= tempProjLoc) {
			if (p[0] == slash) {
				if (!lastSlash) {
					lastSlash = p;
				}
				else
				if (!secondToLastSlash) {
					secondToLastSlash = p;
					break;
				}
				else {
					dd_log("error getting project path");
					return 0;
				}
			}
			p--;
		}
		if (!secondToLastSlash) {
			dd_log("avdl error: can't truncate path of cengine");
			return 0;
		}
		(secondToLastSlash+1)[0] = '\0';
		dd_log("final location: %s", tempProjLoc);
		#endif
	}
	// error
	else {
		return 0;
	}

	return tempProjLoc;

}

void avdl_initProjectLocation() {
}

void avdl_cleanProjectLocation() {
}
