#include "avdl_ray3.h"
#include "avdl_vec3.h"
#include "avdl_vec4.h"
#include "shared/avdl_log.h"

void avdl_ray3_create(struct avdl_ray3 *o) {
	avdl_vec3_Setf(&o->position, 0, 0, 0);
	avdl_vec3_Setf(&o->direction, 0, 0, -1);
}

void avdl_ray3_SetPosition(struct avdl_ray3 *o, struct avdl_vec3 *position) {
	avdl_ray3_SetPosition3f(o, avdl_vec3_X(position), avdl_vec3_Y(position), avdl_vec3_Z(position));
}

void avdl_ray3_SetPositionVec4(struct avdl_ray3 *o, struct avdl_vec4 *position) {
	avdl_ray3_SetPosition3f(o, avdl_vec4_X(position), avdl_vec4_Y(position), avdl_vec4_Z(position));
}

void avdl_ray3_SetPosition3f(struct avdl_ray3 *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->position, x, y, z);
}

struct avdl_vec3 *avdl_ray3_GetPosition(struct avdl_ray3 *o) {
	return &o->position;
}

void avdl_ray3_SetDirection(struct avdl_ray3 *o, struct avdl_vec3 *direction) {
	avdl_ray3_SetDirection3f(o, avdl_vec3_X(direction), avdl_vec3_Y(direction), avdl_vec3_Z(direction));
}

void avdl_ray3_SetDirectionVec4(struct avdl_ray3 *o, struct avdl_vec4 *direction) {
	avdl_ray3_SetDirection3f(o, avdl_vec4_X(direction), avdl_vec4_Y(direction), avdl_vec4_Z(direction));
}

void avdl_ray3_SetDirection3f(struct avdl_ray3 *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->direction, x, y, z);
}

struct avdl_vec3 *avdl_ray3_GetDirection(struct avdl_ray3 *o) {
	return &o->direction;
}

void avdl_ray3_Print(struct avdl_ray3 *o) {
	avdl_log("Ray:");
	avdl_vec3_Print(&o->position);
	avdl_vec3_Print(&o->direction);
}
