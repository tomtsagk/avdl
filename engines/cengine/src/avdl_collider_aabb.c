#include "avdl_collider_aabb.h"

void avdl_collider_aabb_create(struct avdl_collider_aabb *o) {
	avdl_collider_create(&o->parent);
	o->parent.type = AVDL_COLLIDER_TYPE_AABB;
	avdl_vec3_Setf(&o->min, -0.5, -0.5, -0.5);
	avdl_vec3_Setf(&o->max,  0.5,  0.5,  0.5);

	avdl_mesh_create(&o->mesh);
	avdl_mesh_set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX);
	avdl_mesh_set_colour(&o->mesh, 0, 0, 0);
	o->mesh.draw_type = 1;

	o->setMin = avdl_collider_aabb_setMin;
	o->setMax = avdl_collider_aabb_setMax;

	o->getMaxX = avdl_collider_aabb_getMaxX;
	o->getMaxY = avdl_collider_aabb_getMaxY;
	o->getMaxZ = avdl_collider_aabb_getMaxZ;

	o->draw = avdl_collider_aabb_draw;
}

void avdl_collider_aabb_clean(struct avdl_collider_aabb *o) {
	avdl_mesh_clean(&o->mesh);
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

void avdl_collider_aabb_draw(struct avdl_collider_aabb *o) {

	dd_matrix_push();
	dd_translatef(
		avdl_vec3_X(&o->min) +(avdl_vec3_X(&o->max) -avdl_vec3_X(&o->min))/2,
		avdl_vec3_Y(&o->min) +(avdl_vec3_Y(&o->max) -avdl_vec3_Y(&o->min))/2,
		avdl_vec3_Z(&o->min) +(avdl_vec3_Z(&o->max) -avdl_vec3_Z(&o->min))/2
	);
	dd_scalef(
		avdl_vec3_X(&o->max) -avdl_vec3_X(&o->min),
		avdl_vec3_Y(&o->max) -avdl_vec3_Y(&o->min),
		avdl_vec3_Z(&o->max) -avdl_vec3_Z(&o->min)
	);
	avdl_mesh_draw(&o->mesh);
	dd_matrix_pop();
}
