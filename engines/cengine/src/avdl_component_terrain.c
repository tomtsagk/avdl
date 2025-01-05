#include "avdl_component_terrain.h"
#include "avdl_component_mesh.h"
#include "avdl_log.h"
#include "avdl_node.h"
#include "dd_math.h"

void avdl_component_terrain_create(struct avdl_component_terrain *o) {
	dd_log("init terrain");
	avdl_component_create(o);

	o->draw = avdl_component_terrain_draw;
	o->GetTerrain = avdl_component_terrain_GetTerrain;
	o->IsOnTerrain = avdl_component_terrain_IsOnTerrain;
	o->GetSpot = avdl_component_terrain_GetSpot;
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

struct avdl_terrain *avdl_component_terrain_GetTerrain(struct avdl_component_terrain *o) {
	return &o->terrain;
}

int avdl_component_terrain_IsOnTerrain(struct avdl_component_terrain *o, struct avdl_node *n) {

	if (!o->terrain.loaded) {
		avdl_log("terrain not loaded yet");
		return 0;
	}

	if (!o->parent.node) {
		avdl_logError("terrain component not attached to any node");
		return 0;
	}

	struct avdl_transform *terrain_transform = o->parent.node->GetLocalTransform(o->parent.node);

	struct dd_vec4 min;
	dd_vec4_set(&min, 0, 0, 0, 1);
	dd_vec4_multiply(&min, o->parent.node->GetGlobalMatrix(o->parent.node));
	struct dd_vec4 max;
	dd_vec4_set(&max, o->terrain.width, 0, o->terrain.height, 1);
	dd_vec4_multiply(&max, o->parent.node->GetGlobalMatrix(o->parent.node));

	struct dd_vec4 point;
	dd_vec4_set(&point, 0, 0, 0, 1);
	dd_vec4_multiply(&point, n->GetGlobalMatrix(n));

	if (point.cell[0] >= min.cell[0] && point.cell[0] <= max.cell[0]
	&&  -point.cell[2] >= min.cell[2] && -point.cell[2] <= max.cell[2]) {
		return 1;
	}

	return 0;
}

float avdl_component_terrain_GetSpot(struct avdl_component_terrain *o, struct avdl_node *n) {

	if (!o->terrain.loaded) {
		avdl_log("terrain not loaded yet");
		return 0;
	}

	if (!o->parent.node) {
		avdl_logError("terrain component not attached to any node");
		return 0;
	}

	struct avdl_transform *terrain_transform = o->parent.node->GetLocalTransform(o->parent.node);

	struct dd_vec4 min;
	dd_vec4_set(&min, 0, 0, 0, 1);
	dd_vec4_multiply(&min, o->parent.node->GetGlobalMatrix(o->parent.node));
	struct dd_vec4 max;
	dd_vec4_set(&max, o->terrain.width, 0, o->terrain.height, 1);
	dd_vec4_multiply(&max, o->parent.node->GetGlobalMatrix(o->parent.node));

	struct dd_vec4 point;
	dd_vec4_set(&point, 0, 0, 0, 1);
	dd_vec4_multiply(&point, n->GetGlobalMatrix(n));

	// player's tile
	int tileX = dd_math_max(dd_math_min(point.cell[0] -min.cell[0], o->terrain.width  -2), 0.0);
	int tileZ = dd_math_max(dd_math_min(-point.cell[2] -min.cell[2], o->terrain.height -2), 0.0);

	// tile index
	int index = ((tileZ *o->terrain.width) +tileX);
        int indexRight = index +1;
	int indexTop = index +o->terrain.width;
	int indexTopRight = index +o->terrain.width +1;

	// interpolate to find terrain's height at given position
        float factorX = point.cell[0] -min.cell[0] -tileX;
        float factorZ = -point.cell[2] -min.cell[2] -tileZ;

	float h_bottom = o->terrain.heights[index] +((o->terrain.heights[indexRight] -o->terrain.heights[index]) *factorX);
	float h_top = o->terrain.heights[indexTop] +((o->terrain.heights[indexTopRight] -o->terrain.heights[indexTop]) *factorX);

	float h_final = h_bottom +((h_top -h_bottom) *factorZ);
	return h_final +min.cell[1];
}
