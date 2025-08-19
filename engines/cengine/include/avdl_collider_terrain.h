#ifndef AVDL_COLLIDER_TERRAIN_H
#define AVDL_COLLIDER_TERRAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_collider.h"
#include "avdl_vec3.h"
#include "avdl_terrain.h"

struct avdl_collider_terrain {
	struct avdl_collider parent;

	struct avdl_terrain *terrain;
};

void avdl_collider_terrain_create(struct avdl_collider_terrain *o);
void avdl_collider_terrain_clean(struct avdl_collider_terrain *o);

void avdl_collider_terrain_SetTerrain(struct avdl_collider_terrain *o, struct avdl_terrain *terrain);
struct avdl_terrain *avdl_collider_terrain_GetTerrain(struct avdl_collider_terrain *o);

#ifdef __cplusplus
}
#endif

#endif
