#include "dd_vec3.h"

void dd_vec3_create(struct dd_vec3 *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
	o->set = dd_vec3_set;
	o->getX = dd_vec3_getX;
	o->getY = dd_vec3_getY;
	o->getZ = dd_vec3_getZ;
}

void dd_vec3_set(struct dd_vec3 *o, float x, float y, float z) {
	o->x = x;
	o->y = y;
	o->z = z;
}

float dd_vec3_getX(struct dd_vec3 *o) {
	return o->x;
}
float dd_vec3_getY(struct dd_vec3 *o) {
	return o->y;
}
float dd_vec3_getZ(struct dd_vec3 *o) {
	return o->z;
}
