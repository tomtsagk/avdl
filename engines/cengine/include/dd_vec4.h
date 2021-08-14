#ifndef DD_VEC4_H
#define DD_VEC4_H

struct dd_vec4 {
	float x, y, z, w;
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

void dd_vec4_add(struct dd_vec4 *o1, struct dd_vec4 *o2);

#endif
