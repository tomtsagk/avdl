#include "dd_vec4.h"

void dd_vec4_create(struct dd_vec4 *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
	o->w = 0;
	o->set = dd_vec4_set;
	o->getX = dd_vec4_getX;
	o->getY = dd_vec4_getY;
	o->getZ = dd_vec4_getZ;
	o->getW = dd_vec4_getW;
}

void dd_vec4_set(struct dd_vec4 *o, float x, float y, float z, float w) {
	o->x = x;
	o->y = y;
	o->z = z;
	o->w = w;
}

float dd_vec4_getX(struct dd_vec4 *o) {
	return o->x;
}
float dd_vec4_getY(struct dd_vec4 *o) {
	return o->y;
}
float dd_vec4_getZ(struct dd_vec4 *o) {
	return o->z;
}
float dd_vec4_getW(struct dd_vec4 *o) {
	return o->w;
}

void dd_vec4_add(struct dd_vec4 *o1, struct dd_vec4 *o2) {
	o1->x += o2->x;
	o1->y += o2->y;
	o1->z += o2->z;
	o1->w += o2->w;
}
