#include "avdl_collider_aabb.h"

#include <ode/ode.h>
extern dSpaceID avdl_collision_space;

static struct avdl_mesh debugMesh;

void avdl_collider_aabb_create(struct avdl_collider_aabb *o) {
	avdl_collider_create(&o->parent);
	o->parent.type = AVDL_COLLIDER_TYPE_AABB;
	avdl_vec3_Setf(&o->min, -0.5, -0.5, -0.5);
	avdl_vec3_Setf(&o->max,  0.5,  0.5,  0.5);

	// box
	o->parent.geom = dCreateBox(avdl_collision_space, 1.0, 1.0, 1.0);
	dGeomSetPosition(o->parent.geom, 0.0, 0.0, 0.0);
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

float avdl_collider_aabb_getCenterX(struct avdl_collider_aabb *o) {
	return (o->max.x +o->min.x) /2.0f;
}

float avdl_collider_aabb_getCenterY(struct avdl_collider_aabb *o) {
	return (o->max.y +o->min.y) /2.0f;
}

float avdl_collider_aabb_getCenterZ(struct avdl_collider_aabb *o) {
	return (o->max.z +o->min.z) /2.0f;
}

float avdl_collider_aabb_getLengthX(struct avdl_collider_aabb *o) {
	return (-o->min.x +o->max.x) /2.0f;
}

float avdl_collider_aabb_getLengthY(struct avdl_collider_aabb *o) {
	return (-o->min.y +o->max.y) /2.0f;
}

float avdl_collider_aabb_getLengthZ(struct avdl_collider_aabb *o) {
	return (-o->min.z +o->max.z) /2.0f;
}

int avdl_collider_aabb_DrawDebug(struct avdl_collider_aabb *o, struct avdl_node *n) {
	dd_matrix_push();
	dd_scalef(
		avdl_collider_aabb_getLengthX(o),
		avdl_collider_aabb_getLengthY(o),
		avdl_collider_aabb_getLengthZ(o)
	);
	avdl_mesh_draw(&debugMesh);
	dd_matrix_pop();
	return 0;
}

int avdl_collider_aabb_DrawDebugNode(struct avdl_collider_aabb *o, struct avdl_node *n) {
	dd_matrix_push();
	dd_multMatrixf(avdl_node_GetGlobalMatrix(n));
	dd_scalef(
		avdl_collider_aabb_getLengthX(o),
		avdl_collider_aabb_getLengthY(o),
		avdl_collider_aabb_getLengthZ(o)
	);
	avdl_mesh_draw(&debugMesh);
	dd_matrix_pop();
	return 0;
}

void avdl_collider_aabb_init() {
	avdl_mesh_create(&debugMesh);
	avdl_mesh_set_primitive(&debugMesh, AVDL_PRIMITIVE_BOX_WIREFRAME);
	avdl_mesh_set_colour(&debugMesh, 0, 1, 0);
}

void avdl_collider_aabb_deinit() {
	avdl_mesh_clean(&debugMesh);
}
