#ifndef AVDL_COMPONENT_CUSTOM_H
#define AVDL_COMPONENT_CUSTOM_H

#include "avdl_component_custom.h"
#include "avdl_component.h"
#include "avdl_string.h"
#include "dd_dynamic_array.h"

struct avdl_component_custom {

	struct avdl_component parent;

	struct avdl_string name;
	struct dd_dynamic_array values;

	void (*after_create)(struct avdl_component_custom *);
	void (*draw)(struct avdl_component_custom *);

};

void avdl_component_custom_create(struct avdl_component_custom *o);
void avdl_component_custom_clean(struct avdl_component_custom *o);

void avdl_component_custom_SetName(struct avdl_component_custom *o, const char *name);
char *avdl_component_custom_GetName(struct avdl_component_custom *o);

void avdl_component_custom_AddVariableName(struct avdl_component_custom *o, char *value);
void avdl_component_custom_AddVariableValue(struct avdl_component_custom *o, char *value, char *type);

#endif
