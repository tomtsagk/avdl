#include "avdl_vec3.h"
#include "shared/avdl_math.h"
#include "shared/avdl_log.h"
#include "dd_matrix.h"
#include "avdl_vec4.h"

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

void avdl_vec3_Set1f(struct avdl_vec3 *o, float val) {
	o->x = val;
	o->y = val;
	o->z = val;
}

void avdl_vec3_SetVec4(struct avdl_vec3 *o, struct avdl_vec4 *v) {
	o->x = avdl_vec4_X(v);
	o->y = avdl_vec4_Y(v);
	o->z = avdl_vec4_Z(v);
}

void avdl_vec3_SetX(struct avdl_vec3 *o, float value) {
	o->x = value;
}

void avdl_vec3_SetY(struct avdl_vec3 *o, float value) {
	o->y = value;
}

void avdl_vec3_SetZ(struct avdl_vec3 *o, float value) {
	o->z = value;
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

void avdl_vec3_Add1f(struct avdl_vec3 *o1, float value) {
	o1->x += value;
	o1->y += value;
	o1->z += value;
}

void avdl_vec3_Add(struct avdl_vec3 *o1, struct avdl_vec3 *o2) {
	o1->x += o2->x;
	o1->y += o2->y;
	o1->z += o2->z;
}

void avdl_vec3_AddVec4(struct avdl_vec3 *o1, struct avdl_vec4 *v) {
	o1->x += avdl_vec4_X(v);
	o1->y += avdl_vec4_Y(v);
	o1->z += avdl_vec4_Z(v);
}

void avdl_vec3_Cross(struct avdl_vec3 *a, struct avdl_vec3 *b) {
	avdl_vec3_Setf(a,
		avdl_vec3_Y(a) *avdl_vec3_Z(b) -avdl_vec3_Z(a) *avdl_vec3_Y(b),
		avdl_vec3_Z(a) *avdl_vec3_X(b) -avdl_vec3_X(a) *avdl_vec3_Z(b),
		avdl_vec3_X(a) *avdl_vec3_Y(b) -avdl_vec3_Y(a) *avdl_vec3_X(b)
	);
}

void avdl_vec3_CrossVec4(struct avdl_vec3 *a, struct avdl_vec4 *b) {
	avdl_vec3_Setf(a,
		avdl_vec3_Y(a) *avdl_vec4_Z(b) -avdl_vec3_Z(a) *avdl_vec4_Y(b),
		avdl_vec3_Z(a) *avdl_vec4_X(b) -avdl_vec3_X(a) *avdl_vec4_Z(b),
		avdl_vec3_X(a) *avdl_vec4_Y(b) -avdl_vec3_Y(a) *avdl_vec4_X(b)
	);
}

void avdl_vec3_Normalise(struct avdl_vec3 *o) {
	float magn = avdl_vec3_Magnitude(o);
	if (magn == 0) {
		return;
	}
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
	return avdl_vec3_X(a) *avdl_vec3_X(b) + avdl_vec3_X(a) *avdl_vec3_Y(b) + avdl_vec3_Z(a) *avdl_vec3_Z(b);
}

float avdl_vec3_DotVec4(struct avdl_vec3 *a, struct avdl_vec4 *b) {
	return avdl_vec3_X(a) *avdl_vec4_X(b) + avdl_vec3_X(a) *avdl_vec4_Y(b) + avdl_vec3_Z(a) *avdl_vec4_Z(b);
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

void avdl_vec3_Subtract1f(struct avdl_vec3 *o, float value) {
	o->x -= value;
	o->y -= value;
	o->z -= value;
}

void avdl_vec3_SubtractVec4(struct avdl_vec3 *o, struct avdl_vec4 *v) {
	o->x -= avdl_vec4_X(v);
	o->y -= avdl_vec4_Y(v);
	o->z -= avdl_vec4_Z(v);
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

void avdl_vec3_Multiply1f(struct avdl_vec3 *o, float x) {
	o->x *= x;
	o->y *= x;
	o->z *= x;
}

void avdl_vec3_MultiplyVec4(struct avdl_vec3 *o, struct avdl_vec4 *v) {
	o->x *= avdl_vec4_X(v);
	o->y *= avdl_vec4_Y(v);
	o->z *= avdl_vec4_Z(v);
}

void avdl_vec3_MultiplyMatrix(struct avdl_vec3 *o, struct dd_matrix *m, float w) {
        struct avdl_vec3 new_vec;

	new_vec.x =
		(o->x *m->cell[(0 %4) +0]) +
		(o->y *m->cell[(0 %4) +4]) +
		(o->z *m->cell[(0 %4) +8]) +
		(w *m->cell[(0 %4) +12]);

	new_vec.y =
		(o->x *m->cell[(1 %4) +0]) +
		(o->y *m->cell[(1 %4) +4]) +
		(o->z *m->cell[(1 %4) +8]) +
		(w *m->cell[(1 %4) +12]);

	new_vec.z =
		(o->x *m->cell[(2 %4) +0]) +
		(o->y *m->cell[(2 %4) +4]) +
		(o->z *m->cell[(2 %4) +8]) +
		(w *m->cell[(2 %4) +12]);

	avdl_vec3_Set(o, &new_vec);
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

void avdl_vec3_Divide1f(struct avdl_vec3 *o, float value) {
	o->x /= value;
	o->y /= value;
	o->z /= value;
}

void avdl_vec3_DivideVec4(struct avdl_vec3 *o, struct avdl_vec4 *v) {
	o->x /= avdl_vec4_X(v);
	o->y /= avdl_vec4_Y(v);
	o->z /= avdl_vec4_Z(v);
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

void avdl_vec3_Invert(struct avdl_vec3 *o) {
	o->x *= -1;
	o->y *= -1;
	o->z *= -1;
}
