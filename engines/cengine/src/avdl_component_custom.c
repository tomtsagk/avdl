#include "avdl_component_custom.h"
#include "avdl_log.h"

#include <stdlib.h>

void avdl_component_custom_create(struct avdl_component_custom *o) {
	avdl_component_create(o);
	o->parent.type = AVDL_COMPONENT_CUSTOM_EDITOR_ENUM;

	avdl_string_create(&o->name, 100);
	dd_da_init(&o->values, sizeof(struct avdl_string *));
}

void avdl_component_custom_clean(struct avdl_component_custom *o) {
}

void avdl_component_custom_SetName(struct avdl_component_custom *o, const char *name) {
	avdl_string_empty(&o->name);
	avdl_string_cat(&o->name, name);
}

char *avdl_component_custom_GetName(struct avdl_component_custom *o) {
	return avdl_string_toCharPtr(&o->name);
}

void avdl_component_custom_AddVariableName(struct avdl_component_custom *o, char *value) {
	struct avdl_string *str = malloc(sizeof(struct avdl_string));
	avdl_string_create(str, 100);
	avdl_string_cat(str, value);
	dd_da_push(&o->values, &str);
}

void avdl_component_custom_AddVariableValue(struct avdl_component_custom *o, char *value, char *type) {
	struct avdl_string *str = malloc(sizeof(struct avdl_string));
	avdl_string_create(str, 100);
	avdl_string_cat(str, value);
	dd_da_push(&o->values, &str);

	struct avdl_string *strType = malloc(sizeof(struct avdl_string));
	avdl_string_create(strType, 100);
	avdl_string_cat(strType, type);
	dd_da_push(&o->values, &strType);
}
