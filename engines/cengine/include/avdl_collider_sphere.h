#ifndef AVDL_COLLIDER_SPHERE_H
#define AVDL_COLLIDER_SPHERE_H

#include "avdl_collider.h"
#include "dd_vec3.h"

struct avdl_collider_sphere {
	struct avdl_collider parent;

	float radius;

	void (*setRadius)(struct avdl_collider_sphere *, float);
};

void avdl_collider_sphere_create(struct avdl_collider_sphere *o);
void avdl_collider_sphere_clean(struct avdl_collider_sphere *o);

void avdl_collider_sphere_setRadius(struct avdl_collider_sphere *o, float);

#endif
