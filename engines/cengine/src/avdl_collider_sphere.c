#include "avdl_collider_sphere.h"

void avdl_collider_sphere_create(struct avdl_collider_sphere *o) {
	avdl_collider_create(&o->parent);

	o->parent.type = AVDL_COLLIDER_TYPE_SPHERE;
	o->radius = 1.0;

	o->setRadius = avdl_collider_sphere_setRadius;
	o->draw = avdl_collider_sphere_draw;
}

void avdl_collider_sphere_clean(struct avdl_collider_sphere *o) {
}

void avdl_collider_sphere_setRadius(struct avdl_collider_sphere *o, float radius) {
	o->radius = radius;
}

void avdl_collider_sphere_draw(struct avdl_collider_sphere *o) {
}
