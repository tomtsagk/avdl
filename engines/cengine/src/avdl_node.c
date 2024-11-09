#include "avdl_node.h"

void avdl_node_create(struct avdl_node *o) {
	o->parent = 0;

	o->GetLocalTransform = avdl_node_GetLocalTransform;
	o->GetGlobalMatrix = avdl_node_GetGlobalMatrix;
	o->AddChild = avdl_node_AddChild;

	dd_matrix_identity(&o->globalMatrix);
	avdl_transform_create(&o->localTransform);

	dd_dynamic_array_create(&o->children);
	dd_da_init(&o->children, sizeof(struct avdl_node));
}

void avdl_node_clean(struct avdl_node *o) {
}

struct avdl_transform *avdl_node_GetLocalTransform(struct avdl_node *o) {
	return &o->localTransform;
}

struct dd_matrix *avdl_node_GetGlobalMatrix(struct avdl_node *o) {
	dd_matrix_identity(&o->globalMatrix);
	if (o->parent) {
		dd_matrix_copy(&o->globalMatrix, avdl_node_GetGlobalMatrix(&o->parent));
	}
	dd_matrix_mult(&o->globalMatrix, avdl_transform_GetMatrix(&o->localTransform));
	return &o->globalMatrix;
}

struct avdl_node *avdl_node_AddChild(struct avdl_node *o) {
	dd_da_pushEmpty(&o->children);
	return dd_da_get(&o->children, -1);
}
