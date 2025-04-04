#ifndef AVDL_VEC4_H
#define AVDL_VEC4_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_matrix.h"
#include "avdl_vec3.h"

struct avdl_vec4 {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
	union {
		float w;
		float a;
	};
};

void avdl_vec4_create(struct avdl_vec4 *o);
void avdl_vec4_Setf(struct avdl_vec4 *o, float x, float y, float z, float w);
void avdl_vec4_Set(struct avdl_vec4 *o, struct avdl_vec4 *o2);
void avdl_vec4_SetVec3(struct avdl_vec4 *o, struct avdl_vec3 *o2);
void avdl_vec4_SetX(struct avdl_vec4 *o, float value);
void avdl_vec4_SetY(struct avdl_vec4 *o, float value);
void avdl_vec4_SetZ(struct avdl_vec4 *o, float value);
void avdl_vec4_SetW(struct avdl_vec4 *o, float value);

// math
void avdl_vec4_Add (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Addf(struct avdl_vec4 *o1, float x, float y, float z, float w);

void avdl_vec4_Subtract (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Subtractf(struct avdl_vec4 *o1, float x, float y, float z, float w);

void avdl_vec4_Multiply (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Multiplyf(struct avdl_vec4 *o, float x, float y, float z, float w);
void avdl_vec4_Multiply1f(struct avdl_vec4 *o, float x);
void avdl_vec4_MultiplyMatrix(struct avdl_vec4 *o, struct dd_matrix *mat);

void avdl_vec4_Divide (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Dividef(struct avdl_vec4 *o, float x, float y, float z, float w);

float avdl_vec4_X(struct avdl_vec4 *o);
float avdl_vec4_Y(struct avdl_vec4 *o);
float avdl_vec4_Z(struct avdl_vec4 *o);
float avdl_vec4_W(struct avdl_vec4 *o);

float avdl_vec4_Dot(struct avdl_vec4 *a, struct avdl_vec4 *b);
void avdl_vec4_Cross(struct avdl_vec4 *a, struct avdl_vec4 *b);
float avdl_vec4_Distance(struct avdl_vec4 *a, struct avdl_vec4 *b);

void avdl_vec4_Print(struct avdl_vec4 *);

void avdl_vec4_Normalise(struct avdl_vec4 *o);
float avdl_vec4_Magnitude(struct avdl_vec4 *o);

#ifdef __cplusplus
}
#endif

#endif
