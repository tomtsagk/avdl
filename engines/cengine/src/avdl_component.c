#include "avdl_component.h"
#include "avdl_log.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

void avdl_component_create(struct avdl_component *o) {
	o->Copy = avdl_component_Copy;
	o->after_create = avdl_component_after_create;
	o->clean = avdl_component_clean;

	o->node = 0;
}

void avdl_component_clean(struct avdl_component *o) {
}

struct avdl_node *avdl_component_GetNode(struct avdl_component *o) {
	return o->node;
}

void avdl_component_after_create(struct avdl_component *o) {
}

int avdl_component_Copy(struct avdl_component *o, struct avdl_component *target) {
	return 0;
}

// Array of modifiable properties
static struct avdl_component_property property_array[] = {
};
static int property_array_count = sizeof(property_array) /sizeof(struct avdl_component_property);;

int avdl_component_SetPropertyInt(struct avdl_component *c, const char *property_name, int value) {
	for (int i = 0; i < property_array_count; i++) {
		if (strcmp(property_name, property_array[i].name) == 0) {
			if (property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_INT) {
				memcpy(((void *)c) +property_array[i].offset, &value, sizeof(int));
				return 0;
			}
			else
			if (property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				float f = value;
				memcpy(((void *)c) +property_array[i].offset, &f, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
	return 0;
}

int avdl_component_SetPropertyFloat(struct avdl_component *c, const char *property_name, float value) {
	for (int i = 0; i < property_array_count; i++) {
		if (strcmp(property_name, property_array[i].name) == 0) {
			if (property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_FLOAT) {
				memcpy(((void *)c) +property_array[i].offset, &value, sizeof(float));
				return 0;
			}
			return -1;
		}
	}
	return 0;
}

int avdl_component_SetPropertyString(struct avdl_component *c, const char *property_name, const char *value) {
	for (int i = 0; i < property_array_count; i++) {
		if (strcmp(property_name, property_array[i].name) == 0) {
			if (property_array[i].type == AVDL_COMPONENT_PROPERTY_TYPE_STRING) {
				char *prop;
				prop = malloc(sizeof(char) *strlen(value));
				strcpy(prop, value);
				memcpy(((void *)c) +property_array[i].offset, &prop, sizeof(char *));
				return 0;
			}
			return -1;
		}
	}
	return 0;
}
