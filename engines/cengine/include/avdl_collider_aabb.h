#ifndef AVDL_COLLIDER_AABB_H
#define AVDL_COLLIDER_AABB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_collider.h"
#include "dd_vec3.h"
#include "dd_meshColour.h"

struct avdl_collider_aabb {
	struct avdl_collider parent;

	struct dd_vec3 min;
	struct dd_vec3 max;

	struct dd_meshColour mesh;

	void (*setMin)(struct avdl_collider_aabb *, float, float, float);
	void (*setMax)(struct avdl_collider_aabb *, float, float, float);

	float (*getMaxX)(struct avdl_collider_aabb *);
	float (*getMaxY)(struct avdl_collider_aabb *);
	float (*getMaxZ)(struct avdl_collider_aabb *);

	void (*draw)(struct avdl_collider_aabb *);
};

void avdl_collider_aabb_create(struct avdl_collider_aabb *o);
void avdl_collider_aabb_clean(struct avdl_collider_aabb *o);

void avdl_collider_aabb_setMin(struct avdl_collider_aabb *o, float, float, float);
void avdl_collider_aabb_setMax(struct avdl_collider_aabb *o, float, float, float);

float avdl_collider_aabb_getMaxX(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMaxY(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMaxZ(struct avdl_collider_aabb *o);

void avdl_collider_aabb_draw(struct avdl_collider_aabb *o);

#ifdef __cplusplus
}
#endif

#endif
