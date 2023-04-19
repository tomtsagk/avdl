#ifndef DD_VEC4_H
#define DD_VEC4_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_matrix.h"

struct dd_vec4 {
	float cell[4];
	void (*set)(struct dd_vec4 *, float, float, float, float);
	float (*getX)(struct dd_vec4 *);
	float (*getY)(struct dd_vec4 *);
	float (*getZ)(struct dd_vec4 *);
	float (*getW)(struct dd_vec4 *);
};

void dd_vec4_create(struct dd_vec4 *o);
void dd_vec4_set(struct dd_vec4 *o, float x, float y, float z, float w);

float dd_vec4_getX(struct dd_vec4 *o);
float dd_vec4_getY(struct dd_vec4 *o);
float dd_vec4_getZ(struct dd_vec4 *o);
float dd_vec4_getW(struct dd_vec4 *o);

void dd_vec4_clean(struct dd_vec4 *o);

void dd_vec4_add(struct dd_vec4 *o1, float x, float y, float z, float w);
void dd_vec4_addVec3(struct dd_vec4 *o1, struct dd_vec4 *o2);

void dd_vec4_multiply(struct dd_vec4 *o, struct dd_matrix *m);

#ifdef __cplusplus
}
#endif

#endif
