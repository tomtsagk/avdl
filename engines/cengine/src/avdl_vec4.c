#include "avdl_vec4.h"
#include "dd_math.h"
#include "avdl_log.h"

void avdl_vec4_create(struct avdl_vec4 *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
	o->w = 0;
}

void avdl_vec4_Setf(struct avdl_vec4 *o, float x, float y, float z, float w) {
	o->x = x;
	o->y = y;
	o->z = z;
	o->w = w;
}

void avdl_vec4_Set(struct avdl_vec4 *o1, struct avdl_vec4 *o2) {
	if (!o2) {
		avdl_vec4_Setf(o1, 0, 0, 0, 0);
		return;
	}
	o1->x = o2->x;
	o1->y = o2->y;
	o1->z = o2->z;
	o1->w = o2->w;
}

void avdl_vec4_SetVec3(struct avdl_vec4 *o1, struct avdl_vec3 *o2) {
	if (!o2) {
		avdl_vec4_Setf(o1, 0, 0, 0, 0);
		return;
	}
	o1->x = avdl_vec3_X(o2);
	o1->y = avdl_vec3_Y(o2);
	o1->z = avdl_vec3_Z(o2);
	o1->w = 0;
}

void avdl_vec4_SetX(struct avdl_vec4 *o, float value) {
	o->x = value;
}

void avdl_vec4_SetY(struct avdl_vec4 *o, float value) {
	o->y = value;
}

void avdl_vec4_SetZ(struct avdl_vec4 *o, float value) {
	o->z = value;
}

void avdl_vec4_SetW(struct avdl_vec4 *o, float value) {
	o->w = value;
}

float avdl_vec4_X(struct avdl_vec4 *o) {
	return o->x;
}
float avdl_vec4_Y(struct avdl_vec4 *o) {
	return o->y;
}
float avdl_vec4_Z(struct avdl_vec4 *o) {
	return o->z;
}
float avdl_vec4_W(struct avdl_vec4 *o) {
	return o->w;
}

void avdl_vec4_Addf(struct avdl_vec4 *o1, float x, float y, float z, float w) {
	o1->x += x;
	o1->y += y;
	o1->z += z;
	o1->w += w;
}

void avdl_vec4_Add(struct avdl_vec4 *o1, struct avdl_vec4 *o2) {
	o1->x += o2->x;
	o1->y += o2->y;
	o1->z += o2->z;
	o1->w += o2->w;
}

void avdl_vec4_Subtractf(struct avdl_vec4 *o1, float x, float y, float z, float w) {
	o1->x -= x;
	o1->y -= y;
	o1->z -= z;
	o1->w -= w;
}

void avdl_vec4_Subtract(struct avdl_vec4 *o1, struct avdl_vec4 *o2) {
	o1->x -= o2->x;
	o1->y -= o2->y;
	o1->z -= o2->z;
	o1->w -= o2->w;
}

void avdl_vec4_Multiply (struct avdl_vec4 *o1, struct avdl_vec4 *o2) {
	o1->x = o2->x;
	o1->y = o2->y;
	o1->z = o2->z;
	o1->w = o2->w;
}

void avdl_vec4_Multiplyf(struct avdl_vec4 *o, float x, float y, float z, float w) {
	o->x *= x;
	o->y *= y;
	o->z *= z;
	o->w *= w;
}

void avdl_vec4_Multiply1f(struct avdl_vec4 *o, float f) {
	o->x *= f;
	o->y *= f;
	o->z *= f;
	o->w *= f;
}

void avdl_vec4_Divide (struct avdl_vec4 *o1, struct avdl_vec4 *o2) {
	o1->x /= o2->x;
	o1->y /= o2->y;
	o1->z /= o2->z;
	o1->w /= o2->w;
}

void avdl_vec4_Dividef(struct avdl_vec4 *o, float x, float y, float z, float w) {
	o->x /= x;
	o->y /= y;
	o->z /= z;
	o->w /= w;
}

void avdl_vec4_MultiplyMatrix(struct avdl_vec4 *o, struct dd_matrix *m) {
        struct avdl_vec4 new_vec;

	new_vec.x =
		(o->x *m->cell[(0 %4) +0]) +
		(o->y *m->cell[(0 %4) +4]) +
		(o->z *m->cell[(0 %4) +8]) +
		(o->w *m->cell[(0 %4) +12]);

	new_vec.y =
		(o->x *m->cell[(1 %4) +0]) +
		(o->y *m->cell[(1 %4) +4]) +
		(o->z *m->cell[(1 %4) +8]) +
		(o->w *m->cell[(1 %4) +12]);

	new_vec.z =
		(o->x *m->cell[(2 %4) +0]) +
		(o->y *m->cell[(2 %4) +4]) +
		(o->z *m->cell[(2 %4) +8]) +
		(o->w *m->cell[(2 %4) +12]);

	new_vec.w =
		(o->x *m->cell[(3 %4) +0]) +
		(o->y *m->cell[(3 %4) +4]) +
		(o->z *m->cell[(3 %4) +8]) +
		(o->w *m->cell[(3 %4) +12]);

	avdl_vec4_Set(o, &new_vec);
}

void avdl_vec4_multiplyFloat(struct avdl_vec4 *o, float value) {
	o->x *= value;
	o->y *= value;
	o->z *= value;
	o->w *= value;
}

float avdl_vec4_Dot(struct avdl_vec4 *a, struct avdl_vec4 *b) {
	return a->x *b->x
		+a->y *b->y
		+a->z *b->z
		+a->w *b->w;
}

void avdl_vec4_Cross(struct avdl_vec4 *a, struct avdl_vec4 *b) {
	avdl_vec4_Setf(a,
		avdl_vec4_Y(a) *avdl_vec4_Z(b) -avdl_vec4_Z(a) *avdl_vec4_Y(b),
		avdl_vec4_Z(a) *avdl_vec4_X(b) -avdl_vec4_X(a) *avdl_vec4_Z(b),
		avdl_vec4_X(a) *avdl_vec4_Y(b) -avdl_vec4_Y(a) *avdl_vec4_X(b),
		avdl_vec4_W(a)
	);
}

void avdl_vec4_Print(struct avdl_vec4 *o) {
	avdl_log("avdl_vec4: %f %f %f %f",
		o->x,
		o->y,
		o->z,
		o->w
	);
}

void avdl_vec4_Normalise(struct avdl_vec4 *o) {
	float magn = avdl_vec4_Magnitude(o);
	o->x /= magn;
	o->y /= magn;
	o->z /= magn;
	o->w /= magn;
}

float avdl_vec4_Magnitude(struct avdl_vec4 *o) {
	return dd_math_sqrt(dd_math_pow(o->x, 2) +dd_math_pow(o->y, 2) +dd_math_pow(o->z, 2) +dd_math_pow(o->w, 2));
}

float avdl_vec4_Distance(struct avdl_vec4 *a, struct avdl_vec4 *b) {
	struct avdl_vec4 v;
	avdl_vec4_Setf(&v,
		avdl_vec4_X(b) -avdl_vec4_X(a),
		avdl_vec4_Y(b) -avdl_vec4_Y(a),
		avdl_vec4_Z(b) -avdl_vec4_Z(a),
		avdl_vec4_W(b) -avdl_vec4_W(a)
	);
	return avdl_vec4_Magnitude(&v);
}

void avdl_vec4_Invert(struct avdl_vec4 *o) {
	o->x *= -1;
	o->y *= -1;
	o->z *= -1;
	o->w *= -1;
}
