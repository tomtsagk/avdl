#ifndef AVDL_COMPONENT_MESH_H
#define AVDL_COMPONENT_MESH_H

#include "avdl_component.h"
#include "avdl_mesh.h"
#include "avdl_texture.h"

struct avdl_component_mesh {

	struct avdl_component parent;

	struct avdl_mesh mesh;
	char *mesh_name;

	struct avdl_texture image;
	char *texture_name;

	int hasTransparency;

	int isEditor;

};

void avdl_component_mesh_create(struct avdl_component_mesh *o);
void avdl_component_mesh_clean(struct avdl_component_mesh *o);

void avdl_component_mesh_after_create(struct avdl_component_mesh *o);
void avdl_component_mesh_draw(struct avdl_component_mesh *o);

int avdl_component_mesh_Copy(struct avdl_component *o, struct avdl_component *target);

struct avdl_vec3 *avdl_component_mesh_GetBoundsCenter(struct avdl_component_mesh *o);
struct avdl_vec3 *avdl_component_mesh_GetBoundsExtend(struct avdl_component_mesh *o);

int avdl_component_mesh_SetPropertyInt(struct avdl_component_mesh *c, const char *property_name, int value);
int avdl_component_mesh_SetPropertyFloat(struct avdl_component_mesh *c, const char *property_name, float value);
int avdl_component_mesh_SetPropertyString(struct avdl_component_mesh *c, const char *property_name, const char *value);
int avdl_component_mesh_GetPropertyIndexInt(struct avdl_component_mesh *c, int index);
float avdl_component_mesh_GetPropertyIndexFloat(struct avdl_component_mesh *c, int index);
char *avdl_component_mesh_GetPropertyIndexString(struct avdl_component_mesh *c, int index);

extern struct avdl_component_property avdl_component_mesh_property_array[];
extern int avdl_component_mesh_property_array_count;

#endif
