#ifndef DD_GAME_H
#define DD_GAME_H

#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
#include "avdl_graphics.h"

#ifndef PKG_LOCATION
#define PKG_LOCATION ""
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

// way to change the background clear colour
extern float dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b;
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#define dd_clearColour(r, g, b, a) dd_clearcolor_r = r; dd_clearcolor_g = g; dd_clearcolor_b = b;
#else
#define dd_clearColour(r, g, b, a) dd_clearcolor_r = dd_math_pow(r, 2.2); dd_clearcolor_g = dd_math_pow(g, 2.2); dd_clearcolor_b = dd_math_pow(b, 2.2);
#endif

// initialise all pre-game data
void dd_gameInitDefault();
extern void dd_gameInit();

// game title and function to change it
extern char *gameTitle;
#define dd_setGameTitle(GAME_TITLE) gameTitle = GAME_TITLE;

// game init size and function for it
extern int dd_gameInitWindowWidth, dd_gameInitWindowHeight;
#define dd_setInitWindowSize(w, h) dd_gameInitWindowWidth = w; dd_gameInitWindowHeight = h;

// return screen limits on given `z`
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
extern int dd_width;
extern int dd_height;
#define dd_window_width() dd_width
#define dd_window_height() dd_height
#else
int dd_window_width();
int dd_window_height();
#endif
float dd_screen_width_get (float z);
float dd_screen_height_get(float z);

// return distance from camera, based on given width or height
float dd_screen_distance_getw(float width);
float dd_screen_distance_geth(float height);

extern int dd_flag_initialised;
extern int dd_flag_focused;
extern int dd_flag_updateThread;
extern int dd_flag_exit;

// project settings
#if defined(AVDL_DIRECT3D11)
#define PROJ_LOC_TYPE char
#elif defined(_WIN32) || defined(WIN32)
#define PROJ_LOC_TYPE wchar_t
#else
#define PROJ_LOC_TYPE char
#endif
const PROJ_LOC_TYPE *avdl_getProjectLocation();

#define avdl_exit() dd_flag_exit = 1;

/*
 * Two types of package location:
 *
 * Dynamic: The project lives in a directory,
 * which might be moved around.
 *
 * Fixed: The project is permanently in a directory,
 * most likely because it was installed on a system.
 */
enum GAME_PKG_LOCATION_TYPE {
	GAME_PKG_LOCATION_TYPE_DYNAMIC,
	GAME_PKG_LOCATION_TYPE_FIXED,
};

enum GAME_PKG_DEPENDENCIES_TYPE {
	GAME_PKG_DEPENDENCIES_TYPE_DYNAMIC,
	GAME_PKG_DEPENDENCIES_TYPE_FIXED,
};

#ifdef __cplusplus
}
#endif

#endif
