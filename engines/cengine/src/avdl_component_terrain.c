#include "avdl_component_terrain.h"
#include "avdl_component_mesh.h"
#include "avdl_log.h"
#include "avdl_node.h"

void avdl_component_terrain_create(struct avdl_component_terrain *o) {
	dd_log("init terrain");
	avdl_component_create(o);

	o->draw = avdl_component_terrain_draw;
	o->parent.after_create = avdl_component_terrain_after_create;
	o->parent.type = AVDL_COMPONENT_TERRAIN_ENUM;
	o->asset_name = 0;
	o->isEditor = 0;

	avdl_terrain_create(&o->terrain);
}

void avdl_component_terrain_clean(struct avdl_component_terrain *o) {
}

void avdl_component_terrain_after_create(struct avdl_component_terrain *o) {
	if (o->asset_name) {
		if (o->isEditor) {
			avdl_terrain_loadLocal(&o->terrain, o->asset_name);
		}
		else {
			o->terrain.load(&o->terrain, o->asset_name);
		}
	}

	/*
	if (o->parent.node) {
		struct avdl_component_mesh *mesh = avdl_node_GetComponent(o->parent.node, AVDL_COMPONENT_MESH_ENUM);

		if (mesh) {
			avdl_terrain_setMesh(o, &mesh->mesh);
		}
		else {
			avdl_log("terrain found without a mesh component");
		}
	}
	*/
/*
	else {
		o->mesh.set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX);
		o->mesh.set_colour(&o->mesh, 1.0, 0.0, 1.0);
	}
	*/
}

void avdl_component_terrain_draw(struct avdl_component_terrain *o) {
	o->terrain.draw(&o->terrain);
}
