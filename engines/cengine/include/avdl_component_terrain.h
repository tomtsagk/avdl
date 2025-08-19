#ifndef AVDL_COMPONENT_TERRAIN_H
#define AVDL_COMPONENT_TERRAIN_H

#include "avdl_component.h"
#include "avdl_transform.h"
#include "avdl_terrain.h"
#include "avdl_texture.h"
#include "avdl_shaders.h"
#include "avdl_collider_terrain.h"

struct avdl_component_terrain {

	struct avdl_component parent;

	struct avdl_terrain terrain;
	struct avdl_collider_terrain collider;
	char *asset_name;

	struct avdl_texture img;
	struct avdl_texture img_extra_0;
	struct avdl_texture img_extra_1;
	struct avdl_texture img_extra_2;
	struct avdl_texture img_extra_3;

	char *texture_main_name;
	char *texture0_name;
	char *texture1_name;
	char *texture2_name;
	char *texture3_name;

	float scaleZ;

	int terrainRepeat;

	int isEditor;

};

void avdl_component_terrain_create(struct avdl_component_terrain *o);
void avdl_component_terrain_clean(struct avdl_component_terrain *o);

void avdl_component_terrain_after_create(struct avdl_component_terrain *o);
void avdl_component_terrain_draw(struct avdl_component_terrain *o);

struct avdl_terrain *avdl_component_terrain_GetTerrain(struct avdl_component_terrain *o);

int avdl_component_terrain_IsOnTerrain(struct avdl_component_terrain *o, struct avdl_node *n);
float avdl_component_terrain_GetSpot(struct avdl_component_terrain *o, struct avdl_node *n);
struct avdl_collider *avdl_component_terrain_GetCollider(struct avdl_component_terrain *o);

int avdl_component_terrain_Copy(struct avdl_component *o, struct avdl_component *target);

int avdl_component_terrain_SetPropertyInt(struct avdl_component_terrain *c, const char *property_name, int value);
int avdl_component_terrain_SetPropertyFloat(struct avdl_component_terrain *c, const char *property_name, float value);
int avdl_component_terrain_SetPropertyString(struct avdl_component_terrain *c, const char *property_name, const char *value);
int avdl_component_terrain_GetPropertyIndexInt(struct avdl_component_terrain *c, int index);
float avdl_component_terrain_GetPropertyIndexFloat(struct avdl_component_terrain *c, int index);
char *avdl_component_terrain_GetPropertyIndexString(struct avdl_component_terrain *c, int index);

extern struct avdl_component_property avdl_component_terrain_property_array[];
extern int avdl_component_terrain_property_array_count;

#endif
