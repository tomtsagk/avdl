#ifndef DD_VEC3_H
#define DD_VEC3_H

#ifdef __cplusplus
extern "C" {
#endif

struct dd_vec3 {
	float x, y, z;
	void (*setf)(struct dd_vec3 *, float, float, float);
	float (*getX)(struct dd_vec3 *);
	float (*getY)(struct dd_vec3 *);
	float (*getZ)(struct dd_vec3 *);
};

void dd_vec3_create(struct dd_vec3 *o);

void dd_vec3_set(struct dd_vec3 *o, struct dd_vec3 *src);
void dd_vec3_setf(struct dd_vec3 *o, float x, float y, float z);

void dd_vec3_add(struct dd_vec3 *o, struct dd_vec3 *a, struct dd_vec3 *b);
void dd_vec3_addf(struct dd_vec3 *o1, float x, float y, float z);

void dd_vec3_substract(struct dd_vec3 *o, struct dd_vec3 *a, struct dd_vec3 *b);
void dd_vec3_substractf(struct dd_vec3 *o, float x, float y, float z);

float dd_vec3_getX(struct dd_vec3 *o);
float dd_vec3_getY(struct dd_vec3 *o);
float dd_vec3_getZ(struct dd_vec3 *o);

void dd_vec3_clean(struct dd_vec3 *o);

void dd_vec3_cross(struct dd_vec3 *o, struct dd_vec3 *v1, struct dd_vec3 *v2);
float dd_vec3_dot(struct dd_vec3 *a, struct dd_vec3 *b);
void dd_vec3_normalise(struct dd_vec3 *o);
float dd_vec3_magnitude(struct dd_vec3 *o);

float dd_vec3_rotateX(struct dd_vec3 *o, float rad);
float dd_vec3_rotateY(struct dd_vec3 *o, float rad);
float dd_vec3_rotateZ(struct dd_vec3 *o, float rad);

void dd_vec3_print(struct dd_vec3 *);

#ifdef __cplusplus
}
#endif

#endif
