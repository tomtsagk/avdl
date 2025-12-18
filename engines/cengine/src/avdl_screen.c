#include "avdl_screen.h"
#include "dd_game.h"
#include "dd_fov.h"
#include "shared/avdl_math.h"
#include "avdl_vec3.h"
#include "avdl_ray3.h"
#include "avdl_engine.h"

extern struct avdl_engine engine;

int avdl_screen_GetWidth () {
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
	return dd_width;
	#else
	int w, h;
	SDL_GetWindowSize(engine.graphics.sdl_window, &w, &h);
	return w;
	#endif
}

int avdl_screen_GetHeight() {
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
	return dd_height;
	#else
	int w, h;
	SDL_GetWindowSize(engine.graphics.sdl_window, &w, &h);
	return h;
	#endif
}

// screen limits
float dd_screen_width_get (float z) {
	if (avdl_screen_GetWidth() > avdl_screen_GetHeight()) {
		return dd_screen_height_get(z) *dd_fovaspect_get();
	}
	else {
		return dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) ) *z *2;
	}
}

float dd_screen_height_get(float z) {
	if (avdl_screen_GetWidth() > avdl_screen_GetHeight()) {
		return dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) ) *z *2;
	}
	else {
		return dd_screen_width_get(z) *dd_fovaspect_get();
	}
}

float dd_screen_distance_getw(float width) {
	if (avdl_screen_GetWidth() > avdl_screen_GetHeight()) {
		return dd_screen_distance_geth(width /dd_fovaspect_get());
	}
	else {
		return (width/2) /dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) );
	}
}

float dd_screen_distance_geth(float height) {
	if (avdl_screen_GetWidth() > avdl_screen_GetHeight()) {
		return (height/2) /dd_math_tan( dd_math_dec2rad(dd_fovy_get() /2) );
	}
	else {
		return dd_screen_distance_getw(height /dd_fovaspect_get());
	}
}


// Get a ray from the camera, based on the given screen position
void avdl_screen_ScreenPositionToRay(float positionX, float positionY, struct avdl_ray3 *outray) {

	// arbitrary distance from camera to do calculations
	float distanceFromCamera = 5;

	// screen size to float
	float screenWidth = avdl_screen_GetWidth();
	float screenHeight = avdl_screen_GetHeight();

	// convert pixel coordinates to [-1, 1]
	float npositionX = ((positionX /screenWidth ) *2.0) -1.0;
	float npositionY = ((positionY /screenHeight) *2.0) -1.0;

	// scale the biggest screen dimention, based on aspect ratio
	// so if width = 8 and height = 4, then width will be in range of [-2, 2]
	if (screenWidth < screenHeight) {
		float aspectRatio;
		aspectRatio = screenHeight /screenWidth;
		npositionY *= aspectRatio;
	}
	else {
		float aspectRatio;
		aspectRatio = screenWidth /screenHeight;
		npositionX *= aspectRatio;
	}

	// find screen square size
	float fovhalf = dd_fovy_get() /2.0;
	float screenSquareSize = distanceFromCamera *dd_math_tan(dd_math_dec2rad(fovhalf));

	// final position (camera space)
	struct avdl_vec3 pos;
	avdl_vec3_Setf(&pos,
		npositionX *screenSquareSize,
		-(npositionY *screenSquareSize),
		-distanceFromCamera
	);
	avdl_vec3_Normalise(&pos);

	avdl_ray3_SetPosition3f(outray, 0, 0, 0);
	avdl_ray3_SetDirection(outray, &pos);
}

