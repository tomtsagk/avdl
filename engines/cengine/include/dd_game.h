#ifndef DD_GAME_H
#define DD_GAME_H

#include <SDL2/SDL.h>

/* defines game-specific data that are used by the engine
 * but are configured from each individual game
 */
extern SDL_Window* mainWindow;
extern SDL_GLContext mainGLContext;
extern SDL_TimerID timer;

// way to change the background clear colour
extern float dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b;
//#define dd_clearColour(r, g, b, a) glClearColor(r, g, b, a)
#define dd_clearColour(r, g, b, a) dd_clearcolor_r = r; dd_clearcolor_g = g; dd_clearcolor_b = b;
#define dd_clearDepth() glClear(GL_DEPTH_BUFFER_BIT)

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
#if DD_PLATFORM_ANDROID
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

// full screen
#if DD_PLATFORM_NATIVE
// temporarily disable
//#define dd_fullscreenToggle() glutFullScreenToggle()
#elif DD_PLATFORM_ANDROID
#define dd_fullscreenToggle()
#endif

extern int dd_flag_initialised;
extern int dd_flag_focused;
extern int dd_flag_updateThread;
extern int dd_flag_exit;

#endif
