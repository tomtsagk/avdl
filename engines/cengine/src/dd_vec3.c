#include "dd_vec3.h"
#include "dd_math.h"

void dd_vec3_create(struct dd_vec3 *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
	//o->set = dd_vec3_set;
	o->setf = dd_vec3_setf;
	o->getX = dd_vec3_getX;
	o->getY = dd_vec3_getY;
	o->getZ = dd_vec3_getZ;
}

void dd_vec3_set(struct dd_vec3 *o, struct dd_vec3 *src) {
	o->x = src->x;
	o->y = src->y;
	o->z = src->z;
}

void dd_vec3_setf(struct dd_vec3 *o, float x, float y, float z) {
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

void dd_vec3_addf(struct dd_vec3 *o1, float x, float y, float z) {
	o1->x += x;
	o1->y += y;
	o1->z += z;
}

void dd_vec3_add(struct dd_vec3 *o, struct dd_vec3 *a, struct dd_vec3 *b) {
	o->x = a->x +b->x;
	o->y = a->y +b->y;
	o->z = a->z +b->z;
}

void dd_vec3_cross(struct dd_vec3 *o, struct dd_vec3 *v1, struct dd_vec3 *v2) {
	dd_vec3_setf(o,
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

float dd_vec3_dot(struct dd_vec3 *a, struct dd_vec3 *b) {
	return a->x *b->x + a->y *b->y + a->z *b->z;
}

void dd_vec3_substract(struct dd_vec3 *o, struct dd_vec3 *a, struct dd_vec3 *b) {
	o->x = a->x -b->x;
	o->y = a->y -b->y;
	o->z = a->z -b->z;
}

void dd_vec3_substractf(struct dd_vec3 *o, float x, float y, float z) {
	o->x -= x;
	o->y -= y;
	o->z -= z;
}
