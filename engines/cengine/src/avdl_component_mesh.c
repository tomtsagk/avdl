#include "avdl_component_mesh.h"
#include "avdl_log.h"

void avdl_component_mesh_create(struct avdl_component_mesh *o) {
	avdl_component_create(o);

	o->draw = avdl_component_mesh_draw;
	o->parent.after_create = avdl_component_mesh_after_create;
	o->parent.type = AVDL_COMPONENT_MESH_ENUM;
	o->mesh_name = 0;
	o->texture_name = 0;
	o->hasTransparency = 0;
	o->isEditor = 0;

	avdl_mesh_create(&o->mesh);
	dd_image_create(&o->image);
}

void avdl_component_mesh_clean(struct avdl_component_mesh *o) {
}

void avdl_component_mesh_after_create(struct avdl_component_mesh *o) {
	if (o->mesh_name) {
		if (o->isEditor) {
			avdl_mesh_loadLocal(&o->mesh, o->mesh_name, DD_PLY);
		}
		else {
			o->mesh.load(&o->mesh, o->mesh_name, DD_PLY);
		}
	}
	else {
		o->mesh.set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX);
		o->mesh.set_colour(&o->mesh, 1.0, 0.0, 1.0);
	}

	if (o->texture_name) {
		if (o->isEditor) {
			dd_image_setLocal(&o->image, o->texture_name, AVDL_IMAGETYPE_PNG);
		}
		else {
			o->image.set(&o->image, o->texture_name, AVDL_IMAGETYPE_PNG);
		}
		o->mesh.setTexture(&o->mesh, &o->image);
	}

	if (o->hasTransparency) {
		o->mesh.setTransparency(&o->mesh, o->hasTransparency);
	}
}

void avdl_component_mesh_draw(struct avdl_component_mesh *o) {
	o->mesh.draw(&o->mesh);
}
