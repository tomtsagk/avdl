#ifndef DD_VEC3_H
#define DD_VEC3_H

struct dd_vec3 {
	float x, y, z;
	void (*set)(struct dd_vec3 *, float, float, float);
	float (*getX)(struct dd_vec3 *);
	float (*getY)(struct dd_vec3 *);
	float (*getZ)(struct dd_vec3 *);
};

void dd_vec3_create(struct dd_vec3 *o);
void dd_vec3_set(struct dd_vec3 *o, float x, float y, float z);
void dd_vec3_add(struct dd_vec3 *o1, struct dd_vec3 *o2);

float dd_vec3_getX(struct dd_vec3 *o);
float dd_vec3_getY(struct dd_vec3 *o);
float dd_vec3_getZ(struct dd_vec3 *o);

void dd_vec3_clean(struct dd_vec3 *o);

#endif
