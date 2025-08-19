#include "avdl_collider_aabb.h"

void avdl_collider_aabb_create(struct avdl_collider_aabb *o) {
	avdl_collider_create(&o->parent);
	o->parent.type = AVDL_COLLIDER_TYPE_AABB;
	avdl_vec3_Setf(&o->min, -0.5, -0.5, -0.5);
	avdl_vec3_Setf(&o->max,  0.5,  0.5,  0.5);

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

float avdl_collider_aabb_getMinX(struct avdl_collider_aabb *o) {
	return o->min.x;
}

float avdl_collider_aabb_getMinY(struct avdl_collider_aabb *o) {
	return o->min.y;
}

float avdl_collider_aabb_getMinZ(struct avdl_collider_aabb *o) {
	return o->min.z;
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
