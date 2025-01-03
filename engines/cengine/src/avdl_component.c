#include "avdl_component.h"
#include "avdl_log.h"

void avdl_component_create(struct avdl_component *o) {
	o->GetNode = avdl_component_GetNode;
	o->SetType = avdl_component_SetType;
	o->after_create = avdl_component_after_create;

	o->type = AVDL_COMPONENT_INAVLID_ENUM;
	o->node = 0;
}

void avdl_component_clean(struct avdl_component *o) {
}

struct avdl_node *avdl_component_GetNode(struct avdl_component *o) {
	return o->node;
}

void avdl_component_SetType(struct avdl_component *o, int type) {
	o->type = AVDL_COMPONENT_CUSTOM_ENUM +type;
}

void avdl_component_after_create(struct avdl_component *o) {
}

int avdl_component_GetType(struct avdl_component *o) {
	return o->type;
}
