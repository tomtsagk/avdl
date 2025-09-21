#include "avdl_component_skinned_mesh.h"
#include "shared/avdl_log.h"
#include <string.h>
#include <stdlib.h>

void avdl_component_skinned_mesh_create(struct avdl_component_skinned_mesh *o) {
	avdl_component_create(o);

	// overrides
	o->parent.after_create = avdl_component_skinned_mesh_after_create;
	o->parent.Copy = avdl_component_skinned_mesh_Copy;
	o->parent.clean = avdl_component_skinned_mesh_clean;

	o->mesh_name = 0;
	o->texture_name = 0;
	o->dirty_mesh_name = 0;
	o->dirty_texture_name = 0;
	o->hasTransparency = 0;
	o->isEditor = 0;

	avdl_skinned_mesh_create(&o->mesh);
	avdl_texture_create(&o->image);
}

void avdl_component_skinned_mesh_clean(struct avdl_component_skinned_mesh *o) {
	avdl_skinned_mesh_clean(&o->mesh);
	avdl_texture_clean(&o->image);

	if (o->dirty_mesh_name && o->mesh_name) {
		free(o->mesh_name);
		o->mesh_name = 0;
	}

	if (o->dirty_texture_name && o->texture_name) {
		free(o->texture_name);
		o->texture_name = 0;
	}

	avdl_component_clean(o);
}

void avdl_component_skinned_mesh_after_create(struct avdl_component_skinned_mesh *o) {
	if (!o->mesh_name) {
		avdl_log("avdl_component_skinned_mesh_after_create: no mesh name");
		if (o->parent.node) {
			struct avdl_node *n = &o->parent.node;
			avdl_log("	-> node name: %s", avdl_node_GetName(n));
		}
		return;
	}
	if (o->mesh_name) {
		if (o->isEditor) {
			avdl_mesh_loadLocal(&o->mesh, o->mesh_name);
		}
		else {
			avdl_mesh_load(&o->mesh, o->mesh_name);
		}
	}
	else {
		//avdl_mesh_set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX);
		//avdl_mesh_set_colour(&o->mesh, 1.0, 0.0, 1.0);
	}

	if (o->texture_name) {
		if (o->isEditor) {
			avdl_texture_LoadExternal(&o->image, o->texture_name);
		}
		else {
			avdl_texture_Load(&o->image, o->texture_name);
		}
		avdl_mesh_setTexture(&o->mesh, &o->image);
	}

	if (o->hasTransparency) {
		avdl_mesh_setTransparency(&o->mesh, o->hasTransparency);
	}
}

void avdl_component_skinned_mesh_draw(struct avdl_component_skinned_mesh *o) {
	//avdl_mesh_draw(&o->mesh);
	avdl_skinned_mesh_draw(&o->mesh);
}

int avdl_component_skinned_mesh_Copy(struct avdl_component *o, struct avdl_component *target) {
	avdl_component_Copy(o, target);
	avdl_log("copying skinned mesh not yet implemented");
	/*
	struct avdl_component_skinned_mesh *m = o;
	struct avdl_component_skinned_mesh *t = target;
	m->mesh_name = t->mesh_name;
	m->texture_name = t->texture_name;
	m->isEditor = t->isEditor;
	*/
	return 0;
}

// Array of modifiable properties
struct avdl_component_property avdl_component_skinned_mesh_property_array[] = {
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_skinned_mesh, mesh_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_skinned_mesh, texture_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_skinned_mesh, hasTransparency, AVDL_COMPONENT_PROPERTY_TYPE_INT),
};
int avdl_component_skinned_mesh_property_array_count = sizeof(avdl_component_skinned_mesh_property_array) /sizeof(struct avdl_component_property);

int avdl_component_skinned_mesh_SetPropertyInt(struct avdl_component_skinned_mesh *c, const char *property_name, int value) {
	if (avdl_component_SetPropertyInt(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_skinned_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_skinned_mesh_property_array[i].name) == 0) {
			if (avdl_component_skinned_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_INT) {
				memcpy(((void *)c) +avdl_component_skinned_mesh_property_array[i].offset, &value, sizeof(int));
				return 0;
			}
			else
			if (avdl_component_skinned_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				float f = value;
				memcpy(((void *)c) +avdl_component_skinned_mesh_property_array[i].offset, &f, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
	return 0;
}

int avdl_component_skinned_mesh_SetPropertyFloat(struct avdl_component_skinned_mesh *c, const char *property_name, float value) {
	if (avdl_component_SetPropertyFloat(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_skinned_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_skinned_mesh_property_array[i].name) == 0) {
			if (avdl_component_skinned_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				memcpy(((void *)c) +avdl_component_skinned_mesh_property_array[i].offset, &value, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
	return 0;
}

int avdl_component_skinned_mesh_SetPropertyString(struct avdl_component_skinned_mesh *c, const char *property_name, const char *value) {
	if (avdl_component_SetPropertyString(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_skinned_mesh_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_skinned_mesh_property_array[i].name) == 0) {
			if (avdl_component_skinned_mesh_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
				char *prop;
				prop = malloc(sizeof(char) *strlen(value) +1);
				strcpy(prop, value);
				strcat(prop, "\0");
				char **p = (char **) ((char *)c +avdl_component_skinned_mesh_property_array[i].offset);
				*p = prop;
				if (strcmp(property_name, "mesh_name") == 0) {
					c->dirty_mesh_name = 1;
				}
				if (strcmp(property_name, "texture_name") == 0) {
					c->dirty_texture_name = 1;
				}
				return 0;
			}
			return -1;
		}
	}
	return 0;
}

int avdl_component_skinned_mesh_GetPropertyIndexInt(struct avdl_component_skinned_mesh *c, int index) {
	if (avdl_component_skinned_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_INT) {
		avdl_log("GetPropertyIndexInt: Failed to get property '%s'", avdl_component_skinned_mesh_property_array[index].name);
		return 0;
	}
	int *p = ((void *) c) +avdl_component_skinned_mesh_property_array[index].offset;
	return *p;
}

float avdl_component_skinned_mesh_GetPropertyIndexFloat(struct avdl_component_skinned_mesh *c, int index) {
	if (avdl_component_skinned_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
		avdl_log("GetPropertyIndexFloat: Failed to get property '%s'", avdl_component_skinned_mesh_property_array[index].name);
		return 0;
	}
	float *p = ((void *) c) +avdl_component_skinned_mesh_property_array[index].offset;
	return *p;
}

char *avdl_component_skinned_mesh_GetPropertyIndexString(struct avdl_component_skinned_mesh *c, int index) {
	if (avdl_component_skinned_mesh_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
		avdl_log("GetPropertyIndexString: Failed to get property '%s'", avdl_component_skinned_mesh_property_array[index].name);
		return 0;
	}
	char **p = ((void *) c) +avdl_component_skinned_mesh_property_array[index].offset;
	return *p;
}

void avdl_component_skinned_mesh_PlayAnimation(struct avdl_component_skinned_mesh *m, const char *name) {
	avdl_skinned_mesh_PlayAnimation(&m->mesh, name);
}

void avdl_component_skinned_mesh_PlayAnimationInstant(struct avdl_component_skinned_mesh *m, const char *name) {
	avdl_skinned_mesh_PlayAnimationInstant(&m->mesh, name);
}

void avdl_component_skinned_mesh_SetOnAnimationDone(struct avdl_component_skinned_mesh *o, void (*func)(void *ctx), void *context) {
	//avdl_log("set on animation done");
	avdl_skinned_mesh_SetOnAnimationDone(&o->mesh, func, context);
}

void avdl_component_skinned_mesh_update(struct avdl_component_skinned_mesh *m, float dt) {
	avdl_skinned_mesh_update(&m->mesh, dt);
}

void avdl_component_skinned_mesh_PrintAnimations(struct avdl_component_skinned_mesh *o) {
	avdl_skinned_mesh_PrintAnimations(&o->mesh);
}
