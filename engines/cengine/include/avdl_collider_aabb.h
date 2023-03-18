#ifndef AVDL_COLLIDER_AABB_H
#define AVDL_COLLIDER_AABB_H

#include "avdl_collider.h"
#include "dd_vec3.h"

struct avdl_collider_aabb {
	struct avdl_collider parent;

	struct dd_vec3 min;
	struct dd_vec3 max;

	void (*setMin)(struct avdl_collider_aabb *, float, float, float);
	void (*setMax)(struct avdl_collider_aabb *, float, float, float);
};

void avdl_collider_aabb_create(struct avdl_collider_aabb *o);
void avdl_collider_aabb_clean(struct avdl_collider_aabb *o);

void avdl_collider_aabb_setMin(struct avdl_collider_aabb *o, float, float, float);
void avdl_collider_aabb_setMax(struct avdl_collider_aabb *o, float, float, float);

#endif
