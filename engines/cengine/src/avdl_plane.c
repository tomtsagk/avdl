#include "avdl_plane.h"
#include "avdl_vec3.h"
#include "avdl_vec4.h"

void avdl_plane_create(struct avdl_plane *o) {
	avdl_vec3_Setf(&o->position, 0, 0, 0);
	avdl_vec3_Setf(&o->normal, 0, 0, -1);
}

void avdl_plane_SetPosition(struct avdl_plane *o, struct avdl_vec3 *position) {
	avdl_plane_SetPosition3f(o, avdl_vec3_X(position), avdl_vec3_Y(position), avdl_vec3_Z(position));
}

void avdl_plane_SetPositionVec4(struct avdl_plane *o, struct avdl_vec4 *position) {
	avdl_plane_SetPosition3f(o, avdl_vec4_X(position), avdl_vec4_Y(position), avdl_vec4_Z(position));
}

void avdl_plane_SetPosition3f(struct avdl_plane *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->position, x, y, z);
}

struct avdl_vec3 *avdl_plane_GetPosition(struct avdl_plane *o) {
	return &o->position;
}

void avdl_plane_SetNormal(struct avdl_plane *o, struct avdl_vec3 *normal) {
	avdl_plane_SetNormal3f(o, avdl_vec3_X(normal), avdl_vec3_Y(normal), avdl_vec3_Z(normal));
}

void avdl_plane_SetNormalVec4(struct avdl_plane *o, struct avdl_vec4 *normal) {
	avdl_plane_SetNormal3f(o, avdl_vec4_X(normal), avdl_vec4_Y(normal), avdl_vec4_Z(normal));
}

void avdl_plane_SetNormal3f(struct avdl_plane *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->normal, x, y, z);
}

struct avdl_vec3 *avdl_plane_GetNormal(struct avdl_plane *o) {
	return &o->normal;
}
