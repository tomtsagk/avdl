#include "avdl_vec3.h"
#include "dd_math.h"
#include "avdl_log.h"

void avdl_vec3_create(struct avdl_vec3 *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
}

void avdl_vec3_Set(struct avdl_vec3 *o, struct avdl_vec3 *src) {
	if (!src) {
		avdl_vec3_Setf(o, 0, 0, 0);
		return;
	}
	o->x = src->x;
	o->y = src->y;
	o->z = src->z;
}

void avdl_vec3_Setf(struct avdl_vec3 *o, float x, float y, float z) {
	o->x = x;
	o->y = y;
	o->z = z;
}

float avdl_vec3_X(struct avdl_vec3 *o) {
	return o->x;
}
float avdl_vec3_Y(struct avdl_vec3 *o) {
	return o->y;
}
float avdl_vec3_Z(struct avdl_vec3 *o) {
	return o->z;
}

void avdl_vec3_Addf(struct avdl_vec3 *o1, float x, float y, float z) {
	o1->x += x;
	o1->y += y;
	o1->z += z;
}

void avdl_vec3_Add(struct avdl_vec3 *o1, struct avdl_vec3 *o2) {
	o1->x += o2->x;
	o1->y += o2->y;
	o1->z += o2->z;
}

void avdl_vec3_Cross(struct avdl_vec3 *o, struct avdl_vec3 *v1, struct avdl_vec3 *v2) {
	avdl_vec3_Setf(o,
		avdl_vec3_Y(v1) *avdl_vec3_Z(v2) -avdl_vec3_Z(v1) *avdl_vec3_Y(v2),
		avdl_vec3_Z(v1) *avdl_vec3_X(v2) -avdl_vec3_X(v1) *avdl_vec3_Z(v2),
		avdl_vec3_X(v1) *avdl_vec3_Y(v2) -avdl_vec3_Y(v1) *avdl_vec3_X(v2)
	);
}

void avdl_vec3_Normalise(struct avdl_vec3 *o) {
	float magn = avdl_vec3_Magnitude(o);
	if (magn == 0) {
		o->x = 0;
		o->y = 1;
		o->z = 0;
		return;
	}
	o->x /= magn;
	o->y /= magn;
	o->z /= magn;
}

float avdl_vec3_Magnitude(struct avdl_vec3 *o) {
	return dd_math_sqrt(dd_math_pow(o->x, 2) +dd_math_pow(o->y, 2) +dd_math_pow(o->z, 2));
}

float avdl_vec3_Dot(struct avdl_vec3 *a, struct avdl_vec3 *b) {
	return a->x *b->x + a->y *b->y + a->z *b->z;
}

void avdl_vec3_Subtract(struct avdl_vec3 *o1, struct avdl_vec3 *o2) {
	o1->x -= o2->x;
	o1->y -= o2->y;
	o1->z -= o2->z;
}

void avdl_vec3_Subtractf(struct avdl_vec3 *o, float x, float y, float z) {
	o->x -= x;
	o->y -= y;
	o->z -= z;
}

void avdl_vec3_Multiply(struct avdl_vec3 *o1, struct avdl_vec3 *o2) {
	o1->x *= o2->x;
	o1->y *= o2->y;
	o1->z *= o2->z;
}

void avdl_vec3_Multiplyf(struct avdl_vec3 *o, float x, float y, float z) {
	o->x *= x;
	o->y *= y;
	o->z *= z;
}

void avdl_vec3_Divide(struct avdl_vec3 *o1, struct avdl_vec3 *o2) {
	o1->x /= o2->x;
	o1->y /= o2->y;
	o1->z /= o2->z;
}

void avdl_vec3_Dividef(struct avdl_vec3 *o, float x, float y, float z) {
	o->x /= x;
	o->y /= y;
	o->z /= z;
}

void avdl_vec3_Print(struct avdl_vec3 *o) {
	avdl_log("avdl_vec3: %f %f %f",
		o->x,
		o->y,
		o->z
	);
}

void avdl_vec3_RotateX(struct avdl_vec3 *o, float rad) {
	float x = o->x;
	float y = o->y;
	float z = o->z;

	o->x = x;
	o->y = y *dd_math_cos(rad) -z *dd_math_sin(rad);
	o->z = y *dd_math_sin(rad) +z *dd_math_cos(rad);
}

void avdl_vec3_RotateY(struct avdl_vec3 *o, float rad) {
	float x = o->x;
	float y = o->y;
	float z = o->z;

	o->x = x *dd_math_cos(rad) +z *dd_math_sin(rad);;
	o->y = y;
	o->z = -x *dd_math_sin(rad) +z *dd_math_cos(rad);
}

void avdl_vec3_RotateZ(struct avdl_vec3 *o, float rad) {
	float x = o->x;
	float y = o->y;
	float z = o->z;

	o->x = x *dd_math_cos(rad) -y *dd_math_sin(rad);;
	o->y = x *dd_math_sin(rad) +y *dd_math_cos(rad);;
	o->z = z;
}
