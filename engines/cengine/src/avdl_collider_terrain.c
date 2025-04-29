#include "avdl_collider_terrain.h"

void avdl_collider_terrain_create(struct avdl_collider_terrain *o) {
	avdl_collider_create(&o->parent);
	o->parent.type = AVDL_COLLIDER_TYPE_TERRRAIN;
	o->terrain = 0;
}

void avdl_collider_terrain_SetTerrain(struct avdl_collider_terrain *o, struct avdl_terrain *terrain) {
	o->terrain = terrain;
}

struct avdl_terrain *avdl_collider_terrain_GetTerrain(struct avdl_collider_terrain *o) {
	return o->terrain;
}
