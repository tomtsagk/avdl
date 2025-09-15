#include "avdl_component_terrain.h"
#include "avdl_component_mesh.h"
#include "shared/avdl_log.h"
#include "avdl_node.h"
#include "dd_math.h"
#include <string.h>
#include <stdlib.h>

void avdl_component_terrain_create(struct avdl_component_terrain *o) {
	avdl_component_create(o);

	o->parent.after_create = avdl_component_terrain_after_create;
	o->asset_name = 0;
	o->texture_main_name = 0;
	o->texture0_name = 0;
	o->texture1_name = 0;
	o->texture2_name = 0;
	o->texture3_name = 0;
	o->isEditor = 0;
	o->parent.Copy = avdl_component_terrain_Copy;
	o->parent.clean = avdl_component_terrain_clean;

	o->scaleZ = 1.0;

	avdl_terrain_create(&o->terrain);
	avdl_texture_create(&o->img);
	avdl_texture_create(&o->img_extra_0);
	avdl_texture_create(&o->img_extra_1);
	avdl_texture_create(&o->img_extra_2);
	avdl_texture_create(&o->img_extra_3);

	o->terrainRepeat = 1;

	avdl_collider_terrain_create(&o->collider);
	avdl_collider_terrain_SetTerrain(&o->collider, &o->terrain);

}

void avdl_component_terrain_clean(struct avdl_component_terrain *o) {
	avdl_terrain_clean(&o->terrain);
	avdl_texture_clean(&o->img);
	avdl_texture_clean(&o->img_extra_0);
	avdl_texture_clean(&o->img_extra_1);
	avdl_texture_clean(&o->img_extra_2);
	avdl_texture_clean(&o->img_extra_3);
	avdl_component_clean(o);
}

void avdl_component_terrain_after_create(struct avdl_component_terrain *o) {
	avdl_terrain_setScaleZ(&o->terrain, o->scaleZ);
	o->terrain.terrainRepeat = o->terrainRepeat;
	if (o->asset_name) {
		if (o->isEditor) {
			avdl_terrain_loadLocal(&o->terrain, o->asset_name);

			if (o->texture_main_name) {
				avdl_texture_setLocal(&o->img, o->texture_main_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img, 0);
			}

			if (o->texture0_name) {
				avdl_texture_setLocal(&o->img_extra_0, o->texture0_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_0, 1);
			}
			if (o->texture1_name) {
				avdl_texture_setLocal(&o->img_extra_1, o->texture1_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_1, 2);
			}
			if (o->texture2_name) {
				avdl_texture_setLocal(&o->img_extra_2, o->texture2_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_2, 3);
			}
			if (o->texture3_name) {
				avdl_texture_setLocal(&o->img_extra_3, o->texture3_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_3, 4);
			}
		}
		else {
			avdl_terrain_load(&o->terrain, o->asset_name);
			if (o->texture_main_name) {
				avdl_texture_set(&o->img, o->texture_main_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img, 0);
			}

			if (o->texture0_name) {
				avdl_texture_set(&o->img_extra_0, o->texture0_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_0, 1);
			}
			if (o->texture1_name) {
				avdl_texture_set(&o->img_extra_1, o->texture1_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_1, 2);
			}
			if (o->texture2_name) {
				avdl_texture_set(&o->img_extra_2, o->texture2_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_2, 3);
			}
			if (o->texture3_name) {
				avdl_texture_set(&o->img_extra_3, o->texture3_name);
				avdl_terrain_setTextureIndex(&o->terrain, &o->img_extra_3, 4);
			}
		}
	}
}

void avdl_component_terrain_draw(struct avdl_component_terrain *o) {
	avdl_terrain_draw(&o->terrain);
}

struct avdl_terrain *avdl_component_terrain_GetTerrain(struct avdl_component_terrain *o) {
	return &o->terrain;
}

int avdl_component_terrain_IsOnTerrain(struct avdl_component_terrain *o, struct avdl_node *n) {

	if (!o->terrain.loaded) {
		//avdl_log("terrain not loaded yet");
		return 0;
	}

	if (!o->parent.node) {
		avdl_log_error("terrain component not attached to any node");
		return 0;
	}

	struct avdl_node *terrain_node = o->parent.node;

	struct avdl_vec4 position;
	avdl_vec4_Setf(&position, 0, 0, 0, 1);
	avdl_vec4_MultiplyMatrix(&position, avdl_node_GetGlobalMatrix(n));
	avdl_vec4_MultiplyMatrix(&position, avdl_node_GetGlobalInverseMatrix(terrain_node));

	return avdl_terrain_isOnTerrain(&o->terrain, avdl_vec4_X(&position), -avdl_vec4_Z(&position));

}

float avdl_component_terrain_GetSpot(struct avdl_component_terrain *o, struct avdl_node *n) {

	if (!o->terrain.loaded) {
		avdl_log("terrain not loaded yet");
		return 0;
	}

	if (!o->parent.node) {
		avdl_log_error("terrain component not attached to any node");
		return 0;
	}

	struct avdl_node *terrain_node = o->parent.node;

	struct avdl_vec4 position;
	avdl_vec4_Setf(&position, 0, 0, 0, 1);
	avdl_vec4_MultiplyMatrix(&position, avdl_node_GetGlobalMatrix(n));
	avdl_vec4_MultiplyMatrix(&position, avdl_node_GetGlobalInverseMatrix(terrain_node));

	return avdl_terrain_getSpot(&o->terrain, avdl_vec4_X(&position), -avdl_vec4_Z(&position));

}

int avdl_component_terrain_Copy(struct avdl_component *o, struct avdl_component *target) {
	avdl_component_Copy(o, target);
	avdl_log("avdl_component_terrain_Copy NOT IMPLEMENTED");
	return 1;
}

// Array of modifiable properties
struct avdl_component_property avdl_component_terrain_property_array[] = {
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, asset_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, texture_main_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, texture0_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, texture1_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, texture2_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, texture3_name, AVDL_COMPONENT_PROPERTY_TYPE_STRING),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, scaleZ, AVDL_COMPONENT_PROPERTY_TYPE_FLOAT),
	AVDL_COMPONENT_PROPERTY_ELEMENT(struct avdl_component_terrain, terrainRepeat, AVDL_COMPONENT_PROPERTY_TYPE_INT),
};
int avdl_component_terrain_property_array_count = sizeof(avdl_component_terrain_property_array) /sizeof(struct avdl_component_property);;

int avdl_component_terrain_SetPropertyInt(struct avdl_component_terrain *c, const char *property_name, int value) {
	if (avdl_component_SetPropertyInt(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_terrain_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_terrain_property_array[i].name) == 0) {
			if (avdl_component_terrain_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_INT) {
				memcpy(((void *)c) +avdl_component_terrain_property_array[i].offset, &value, sizeof(int));
				return 0;
			}
			else
			if (avdl_component_terrain_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				float f = value;
				memcpy(((void *)c) +avdl_component_terrain_property_array[i].offset, &f, sizeof(float));
				return 0;
			}
			avdl_log("failed to set int property '%s', wrong format", property_name);
			return -1;
		}
	}
	avdl_log("could not find int property '%s'", property_name);
	return -1;
}

int avdl_component_terrain_SetPropertyFloat(struct avdl_component_terrain *c, const char *property_name, float value) {
	if (avdl_component_SetPropertyFloat(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_terrain_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_terrain_property_array[i].name) == 0) {
			if (avdl_component_terrain_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				memcpy(((void *)c) +avdl_component_terrain_property_array[i].offset, &value, sizeof(float));
				return 0;
			}
			avdl_log("failed to set float property '%s', wrong format", property_name);
			return -1;
		}
	}

	avdl_log("could not find float property '%s'", property_name);
	return -1;
}

int avdl_component_terrain_SetPropertyString(struct avdl_component_terrain *c, const char *property_name, const char *value) {
	if (avdl_component_SetPropertyString(c, property_name, value) == 0) {
		return 0;
	}
	for (int i = 0; i < avdl_component_terrain_property_array_count; i++) {
		if (strcmp(property_name, avdl_component_terrain_property_array[i].name) == 0) {
			if (avdl_component_terrain_property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
				char *prop;
				prop = malloc(sizeof(char) *(strlen(value)+1));
				strcpy(prop, value);
				strcat(prop, "\0");
				memcpy(((void *)c) +avdl_component_terrain_property_array[i].offset, &prop, sizeof(char *));
				return 0;
			}
			avdl_log("failed to set string property '%s', wrong format", property_name);
			return -1;
		}
	}
	avdl_log("could not find string property '%s'", property_name);
	return -1;
}

int avdl_component_terrain_GetPropertyIndexInt(struct avdl_component_terrain *c, int index) {
	if (avdl_component_terrain_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_INT) {
		avdl_log("GetPropertyIndexInt: Failed to get property '%s'", avdl_component_terrain_property_array[index].name);
		return 0;
	}
	int *p = ((void *) c) +avdl_component_terrain_property_array[index].offset;
	return *p;
}

float avdl_component_terrain_GetPropertyIndexFloat(struct avdl_component_terrain *c, int index) {
	if (avdl_component_terrain_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
		avdl_log("GetPropertyIndexFloat: Failed to get property '%s'", avdl_component_terrain_property_array[index].name);
		return 0;
	}
	float *p = ((void *) c) +avdl_component_terrain_property_array[index].offset;
	return *p;
}

char *avdl_component_terrain_GetPropertyIndexString(struct avdl_component_terrain *c, int index) {
	if (avdl_component_terrain_property_array[index].type != AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
		avdl_log("GetPropertyIndexString: Failed to get property '%s'", avdl_component_terrain_property_array[index].name);
		return 0;
	}
	char **p = ((void *) c) +avdl_component_terrain_property_array[index].offset;
	return *p;
}

struct avdl_collider *avdl_component_terrain_GetCollider(struct avdl_component_terrain *o) {
	return &o->collider;
}
