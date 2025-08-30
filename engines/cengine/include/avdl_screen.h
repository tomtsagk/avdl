#ifndef AVDL_SCREEN_H
#define AVDL_SCREEN_H

#include "avdl_ray3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Contains screen related functions

// Get screen size
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
extern int dd_width;
extern int dd_height;
#endif
int avdl_screen_GetWidth ();
int avdl_screen_GetHeight();

// return screen limits on given `z`
float dd_screen_width_get (float z);
float dd_screen_height_get(float z);

// return distance from camera, based on given width or height
float dd_screen_distance_getw(float width);
float dd_screen_distance_geth(float height);

void avdl_screen_ScreenPositionToRay(float mouseX, float mouseY, struct avdl_ray3 *outray);

#ifdef __cplusplus
}
#endif

#endif
