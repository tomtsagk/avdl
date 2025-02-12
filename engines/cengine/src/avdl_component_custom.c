#include "avdl_component_custom.h"
#include "avdl_log.h"

#include <stdlib.h>

void avdl_component_custom_create(struct avdl_component_custom *o) {
	avdl_component_create(o);
	o->parent.type = AVDL_COMPONENT_CUSTOM_EDITOR_ENUM;
	o->parent.Copy = avdl_component_custom_Copy;

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

int avdl_component_custom_Copy(struct avdl_component *o, struct avdl_component *target) {
	avdl_component_Copy(o, target);
	struct avdl_component_custom *c = o;
	struct avdl_component_custom *t = target;
	avdl_string_copy(&c->name, &t->name);
	for (int i = 0; i < dd_da_count(&t->values); i++) {
		struct avdl_string *str = malloc(sizeof(struct avdl_string));
		struct avdl_string *str_target = dd_da_get(&t->values, i);
		avdl_string_create(str, str_target->maxCharacters);
		avdl_string_copy(str, str_target);
		dd_da_push(&c->values, &str);
	}
	return 0;
}
