#include "dd_vec3.h"
#include "dd_math.h"

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

void dd_vec3_clean(struct dd_vec3 *o) {
}

void dd_vec3_add(struct dd_vec3 *o1, float x, float y, float z) {
	o1->x += x;
	o1->y += y;
	o1->z += z;
}

void dd_vec3_addVec3(struct dd_vec3 *o1, struct dd_vec3 *o2) {
	o1->x += o2->x;
	o1->y += o2->y;
	o1->z += o2->z;
}

void dd_vec3_cross(struct dd_vec3 *o, struct dd_vec3 *v1, struct dd_vec3 *v2) {
	dd_vec3_set(o,
		dd_vec3_getY(v1) *dd_vec3_getZ(v2) -dd_vec3_getZ(v1) *dd_vec3_getY(v2),
		dd_vec3_getZ(v1) *dd_vec3_getX(v2) -dd_vec3_getX(v1) *dd_vec3_getZ(v2),
		dd_vec3_getX(v1) *dd_vec3_getY(v2) -dd_vec3_getY(v1) *dd_vec3_getX(v2)
	);
}

void dd_vec3_normalise(struct dd_vec3 *o) {
	float magn = dd_vec3_magnitude(o);
	o->x /= magn;
	o->y /= magn;
	o->z /= magn;
}

float dd_vec3_magnitude(struct dd_vec3 *o) {
	return dd_math_sqrt(dd_math_pow(o->x, 2) +dd_math_pow(o->y, 2) +dd_math_pow(o->z, 2));
}
