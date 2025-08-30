#ifndef AVDL_PLANE_H
#define AVDL_PLANE_H

#include "avdl_vec3.h"
#include "avdl_vec4.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_plane {
	struct avdl_vec3 position;
	struct avdl_vec3 normal;
};

void avdl_plane_create(struct avdl_plane *o);

// position
void avdl_plane_SetPosition(struct avdl_plane *o, struct avdl_vec3 *position);
void avdl_plane_SetPositionVec4(struct avdl_plane *o, struct avdl_vec4 *position);
void avdl_plane_SetPosition3f(struct avdl_plane *o, float x, float y, float z);
struct avdl_vec3 *avdl_plane_GetPosition(struct avdl_plane *o);

// normal
void avdl_plane_SetNormal(struct avdl_plane *o, struct avdl_vec3 *normal);
void avdl_plane_SetNormalVec4(struct avdl_plane *o, struct avdl_vec4 *normal);
void avdl_plane_SetNormal3f(struct avdl_plane *o, float x, float y, float z);
struct avdl_vec3 *avdl_plane_GetNormal(struct avdl_plane *o);

#ifdef __cplusplus
}
#endif

#endif
