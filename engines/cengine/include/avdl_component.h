#ifndef AVDL_COMPONENT_H
#define AVDL_COMPONENT_H

#include "avdl_node.h"

#include <stddef.h>

enum AVDL_COMPONENT_PROPERTY_TYPE {
	AVDL_COMPONENT_PROPERTY_TYPE_INT,
	AVDL_COMPONENT_PROPERTY_TYPE_FLOAT,
	AVDL_COMPONENT_PROPERTY_TYPE_STRING,
};

struct avdl_component {

	struct avdl_node *node;

	void (*clean)(struct avdl_component *);
	void (*after_create)(struct avdl_component *);
	int (*Copy)(struct avdl_component *, struct avdl_component *);
};

// Store component property data
struct avdl_component_property {
	char *name;
	size_t offset;
	enum AVDL_COMPONENT_PROPERTY_TYPE type;
};

#define AVDL_COMPONENT_PROPERTY_ELEMENT(x, y, z) {#y, offsetof(x, y), z}

void avdl_component_create(struct avdl_component *o);
void avdl_component_clean(struct avdl_component *o);
void avdl_component_after_create(struct avdl_component *o);

struct avdl_node *avdl_component_GetNode(struct avdl_component *o);

int avdl_component_Copy(struct avdl_component *o, struct avdl_component *target);

int avdl_component_SetPropertyInt(struct avdl_component *c, const char *property_name, int value);
int avdl_component_SetPropertyString(struct avdl_component *c, const char *property_name, const char *value);
int avdl_component_SetPropertyFloat(struct avdl_component *c, const char *property_name, float value);

#endif
