#ifndef AVDL_COMPONENT_TERRAIN_H
#define AVDL_COMPONENT_TERRAIN_H

#include "avdl_component.h"
#include "avdl_terrain.h"

struct avdl_component_terrain {

	struct avdl_component parent;

	struct avdl_terrain terrain;
	char *asset_name;

	int isEditor;

	void (*after_create)(struct avdl_component_terrain *);
	void (*draw)(struct avdl_component_terrain *);

};

void avdl_component_terrain_create(struct avdl_component_terrain *o);
void avdl_component_terrain_clean(struct avdl_component_terrain *o);

void avdl_component_terrain_after_create(struct avdl_component_terrain *o);
void avdl_component_terrain_draw(struct avdl_component_terrain *o);

#endif
