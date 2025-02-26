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
	o->parent.Copy = avdl_component_mesh_Copy;

	o->parent.clean = avdl_component_mesh_clean;

	avdl_mesh_create(&o->mesh);
	avdl_texture_create(&o->image);
}

void avdl_component_mesh_clean(struct avdl_component_mesh *o) {
	avdl_mesh_clean(&o->mesh);
	avdl_texture_clean(&o->image);
	avdl_component_clean(o);
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
			avdl_texture_setLocal(&o->image, o->texture_name, AVDL_IMAGETYPE_PNG);
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

int avdl_component_mesh_Copy(struct avdl_component *o, struct avdl_component *target) {
	avdl_component_Copy(o, target);
	struct avdl_component_mesh *m = o;
	struct avdl_component_mesh *t = target;
	m->mesh_name = t->mesh_name;
	m->texture_name = t->texture_name;
	m->isEditor = t->isEditor;
	return 0;
}

// Array of modifiable properties
struct avdl_component_property avdl_component_mesh_property_array[] = {
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_mesh, mesh_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_mesh, texture_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_mesh, hasTransparency, AVDL_COMPONENT_PROPERTY_TYPE_INT),
};
int avdl_component_mesh_property_array_count = sizeof(avdl_component_mesh_property_array) /sizeof(struct avdl_component_property);;

int avdl_component_mesh_SetPropertyInt(struct avdl_component_mesh *c, const char *property_name, int value) {
	if (avdl_component_SetPropertyInt(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_mesh_property_array[i].name) == 0) {
			if (avdl_component_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_INT) {
				memcpy(((void *)c) +avdl_component_mesh_property_array[i].offset, &value, sizeof(int));
				return 0;
			}
			else
			if (avdl_component_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				float f = value;
				memcpy(((void *)c) +avdl_component_mesh_property_array[i].offset, &f, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
}

int avdl_component_mesh_SetPropertyFloat(struct avdl_component_mesh *c, const char *property_name, float value) {
	if (avdl_component_SetPropertyFloat(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_mesh_property_array[i].name) == 0) {
			if (avdl_component_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				memcpy(((void *)c) +avdl_component_mesh_property_array[i].offset, &value, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
}

int avdl_component_mesh_SetPropertyString(struct avdl_component_mesh *c, const char *property_name, const char *value) {
	if (avdl_component_SetPropertyString(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_mesh_property_array[i].name) == 0) {
			if (avdl_component_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
				char *prop;
				prop = malloc(sizeof(char) *strlen(value));
				strcpy(prop, value);
				memcpy(((void *)c) +avdl_component_mesh_property_array[i].offset, &prop, sizeof(char *));
				return 0;
			}
			return -1;
		}
	}
}

int avdl_component_mesh_GetPropertyIndexInt(struct avdl_component_mesh *c, int index) {
	if (avdl_component_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_INT) {
		avdl_log("GetPropertyIndexInt: Failed to get property '%s'", avdl_component_mesh_property_array[index].name);
		return 0;
	}
	int *p = ((void *) c) +avdl_component_mesh_property_array[index].offset;
	return *p;
}

float avdl_component_mesh_GetPropertyIndexFloat(struct avdl_component_mesh *c, int index) {
	if (avdl_component_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
		avdl_log("GetPropertyIndexFloat: Failed to get property '%s'", avdl_component_mesh_property_array[index].name);
		return 0;
	}
	float *p = ((void *) c) +avdl_component_mesh_property_array[index].offset;
	return *p;
}

char *avdl_component_mesh_GetPropertyIndexString(struct avdl_component_mesh *c, int index) {
	if (avdl_component_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
		avdl_log("GetPropertyIndexString: Failed to get property '%s'", avdl_component_mesh_property_array[index].name);
		return 0;
	}
	char **p = ((void *) c) +avdl_component_mesh_property_array[index].offset;
	return *p;
}
