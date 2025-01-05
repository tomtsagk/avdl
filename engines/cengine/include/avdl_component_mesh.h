#ifndef AVDL_COMPONENT_MESH_H
#define AVDL_COMPONENT_MESH_H

#include "avdl_component.h"
#include "avdl_mesh.h"

struct avdl_component_mesh {

	struct avdl_component parent;

	struct avdl_mesh mesh;
	char *mesh_name;

	int isEditor;

	void (*after_create)(struct avdl_component_mesh *);
	void (*draw)(struct avdl_component_mesh *);

};

void avdl_component_mesh_create(struct avdl_component_mesh *o);
void avdl_component_mesh_clean(struct avdl_component_mesh *o);

void avdl_component_mesh_after_create(struct avdl_component_mesh *o);
void avdl_component_mesh_draw(struct avdl_component_mesh *o);

#endif
