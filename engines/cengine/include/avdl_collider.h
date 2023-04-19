#ifndef AVDL_COLLIDER_H
#define AVDL_COLLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_vec3.h"
#include "dd_mesh.h"

#define AVDL_COLLIDER_TYPE_POINT 0
#define AVDL_COLLIDER_TYPE_AABB 1
#define AVDL_COLLIDER_TYPE_OBB 2
#define AVDL_COLLIDER_TYPE_SPHERE 3

struct avdl_collider {
	int type;
};

void avdl_collider_create(struct avdl_collider *o);
void avdl_collider_clean(struct avdl_collider *o);

#ifdef __cplusplus
}
#endif

#endif
