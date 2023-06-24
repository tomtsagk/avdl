#include "avdl_collider_aabb.h"

void avdl_collider_aabb_create(struct avdl_collider_aabb *o) {
	avdl_collider_create(&o->parent);
	o->parent.type = AVDL_COLLIDER_TYPE_AABB;
	dd_vec3_setf(&o->min, -0.5, -0.5, -0.5);
	dd_vec3_setf(&o->max,  0.5,  0.5,  0.5);

	o->setMin = avdl_collider_aabb_setMin;
	o->setMax = avdl_collider_aabb_setMax;

	o->getMaxX = avdl_collider_aabb_getMaxX;
	o->getMaxY = avdl_collider_aabb_getMaxY;
	o->getMaxZ = avdl_collider_aabb_getMaxZ;
}

void avdl_collider_aabb_clean(struct avdl_collider_aabb *o) {
}

void avdl_collider_aabb_setMin(struct avdl_collider_aabb *o, float x, float y, float z) {
	o->min.x = x;
	o->min.y = y;
	o->min.z = z;
}

void avdl_collider_aabb_setMax(struct avdl_collider_aabb *o, float x, float y, float z) {
	o->max.x = x;
	o->max.y = y;
	o->max.z = z;
}

float avdl_collider_aabb_getMaxX(struct avdl_collider_aabb *o) {
	return o->max.x;
}

float avdl_collider_aabb_getMaxY(struct avdl_collider_aabb *o) {
	return o->max.y;
}

float avdl_collider_aabb_getMaxZ(struct avdl_collider_aabb *o) {
	return o->max.z;
}
