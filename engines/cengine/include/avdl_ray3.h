#ifndef AVDL_RAY_H
#define AVDL_RAY_H

#include "avdl_vec3.h"
#include "avdl_vec4.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_ray3 {
	struct avdl_vec3 position;
	struct avdl_vec3 direction;
};

void avdl_ray3_create(struct avdl_ray3 *o);

// position
void avdl_ray3_SetPosition(struct avdl_ray3 *o, struct avdl_vec3 *position);
void avdl_ray3_SetPositionVec4(struct avdl_ray3 *o, struct avdl_vec4 *position);
void avdl_ray3_SetPosition3f(struct avdl_ray3 *o, float x, float y, float z);
struct avdl_vec3 *avdl_ray3_GetPosition(struct avdl_ray3 *o);

// direction
void avdl_ray3_SetDirection(struct avdl_ray3 *o, struct avdl_vec3 *direction);
void avdl_ray3_SetDirectionVec4(struct avdl_ray3 *o, struct avdl_vec4 *direction);
void avdl_ray3_SetDirection3f(struct avdl_ray3 *o, float x, float y, float z);
struct avdl_vec3 *avdl_ray3_GetDirection(struct avdl_ray3 *o);

void avdl_ray3_Print(struct avdl_ray3 *);

#ifdef __cplusplus
}
#endif

#endif
