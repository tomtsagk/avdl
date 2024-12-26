#include "avdl_component.h"
#include "dd_log.h"

void avdl_component_create(struct avdl_component *o) {
	o->GetNode = avdl_component_GetNode;
	o->SetType = avdl_component_SetType;

	o->type = -1;
	o->node = 0;
}

void avdl_component_clean(struct avdl_component *o) {
}

struct avdl_node *avdl_component_GetNode(struct avdl_component *o) {
	return o->node;
}

void avdl_component_SetType(struct avdl_component *o, int type) {
	o->type = type;
}
