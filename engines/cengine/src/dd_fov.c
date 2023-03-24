#include "dd_fov.h"
#include <math.h>

float fovy = 45.0;
float fov_aspect = 0;

void dd_fovy_set(float val) {
	fovy = val;
}

void dd_fovaspect_set(float val) {
	fov_aspect = val;
}

float dd_fovy_get() {return fovy;}
float dd_fovaspect_get() {return fov_aspect;}
