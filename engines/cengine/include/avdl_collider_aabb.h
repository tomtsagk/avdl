#ifndef AVDL_COLLIDER_AABB_H
#define AVDL_COLLIDER_AABB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_collider.h"
#include "avdl_vec3.h"
#include "avdl_mesh.h"

struct avdl_collider_aabb {
	struct avdl_collider parent;

	struct avdl_vec3 min;
	struct avdl_vec3 max;
};

void avdl_collider_aabb_create(struct avdl_collider_aabb *o);
void avdl_collider_aabb_clean(struct avdl_collider_aabb *o);

void avdl_collider_aabb_setMin(struct avdl_collider_aabb *o, float, float, float);
void avdl_collider_aabb_setMax(struct avdl_collider_aabb *o, float, float, float);

float avdl_collider_aabb_getMinX(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMinY(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMinZ(struct avdl_collider_aabb *o);

float avdl_collider_aabb_getMaxX(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMaxY(struct avdl_collider_aabb *o);
float avdl_collider_aabb_getMaxZ(struct avdl_collider_aabb *o);

#ifdef __cplusplus
}
#endif

#endif
