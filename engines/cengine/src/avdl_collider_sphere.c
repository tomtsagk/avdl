#include "avdl_collider_sphere.h"

#include <ode/ode.h>
extern dSpaceID avdl_collision_space;

void avdl_collider_sphere_create(struct avdl_collider_sphere *o) {
	avdl_collider_create(&o->parent);

	o->parent.type = AVDL_COLLIDER_TYPE_SPHERE;
	o->radius = 1.0;

	// Sphere
	o->parent.geom = dCreateSphere(avdl_collision_space, 1.0);
	dGeomSetPosition(o->parent.geom, 0.0, 0.0, 0.0);
}

void avdl_collider_sphere_setRadius(struct avdl_collider_sphere *o, float radius) {
	o->radius = radius;
}
