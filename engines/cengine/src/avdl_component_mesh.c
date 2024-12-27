#include "avdl_component_mesh.h"
#include "dd_log.h"

void avdl_component_mesh_create(struct avdl_component_mesh *o) {
	avdl_component_create(o);

	o->draw = avdl_component_mesh_draw;
	o->parent.after_create = avdl_component_mesh_after_create;
	o->parent.type = AVDL_COMPONENT_MESH_ENUM;
	o->mesh_name = 0;

	avdl_mesh_create(&o->mesh);
}

void avdl_component_mesh_clean(struct avdl_component_mesh *o) {
}

void avdl_component_mesh_after_create(struct avdl_component_mesh *o) {
	if (o->mesh_name) {
		o->mesh.load(&o->mesh, o->mesh_name, DD_PLY);
	}
	else {
		o->mesh.set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX);
		o->mesh.set_colour(&o->mesh, 1.0, 0.0, 1.0);
	}
}

void avdl_component_mesh_draw(struct avdl_component_mesh *o) {
	o->mesh.draw(&o->mesh);
}
