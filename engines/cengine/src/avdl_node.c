#include "avdl_node.h"
#include "dd_log.h"
#include "avdl_component.h"
#include <stdlib.h>
#include <stdio.h>

void avdl_node_create(struct avdl_node *o) {
	o->parent = 0;

	o->GetLocalTransform = avdl_node_GetLocalTransform;
	o->GetGlobalMatrix = avdl_node_GetGlobalMatrix;
	o->AddChild = avdl_node_AddChild;

	dd_matrix_identity(&o->globalMatrix);
	dd_matrix_identity(&o->globalNormalMatrix);
	avdl_transform_create(&o->localTransform);

	dd_dynamic_array_create(&o->components);
	dd_da_init(&o->components, sizeof(struct avdl_component *));

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
		dd_matrix_copy(&o->globalMatrix, avdl_node_GetGlobalMatrix(o->parent));
	}
	dd_matrix_mult(&o->globalMatrix, avdl_transform_GetMatrix(&o->localTransform));
	return &o->globalMatrix;
}

struct dd_matrix *avdl_node_GetGlobalNormalMatrix(struct avdl_node *o) {
	dd_matrix_identity(&o->globalNormalMatrix);
	if (o->parent) {
		dd_matrix_copy(&o->globalNormalMatrix, avdl_node_GetGlobalNormalMatrix(o->parent));
	}
	dd_matrix_mult(&o->globalNormalMatrix, avdl_transform_GetNormalMatrix(&o->localTransform));
	return &o->globalNormalMatrix;
}

struct avdl_node *avdl_node_AddChild(struct avdl_node *o) {
	dd_da_pushEmpty(&o->children);
	struct avdl_node *child = dd_da_get(&o->children, -1);
	avdl_node_create(child);
	child->parent = o;
	return child;
}

struct avdl_component *avdl_node_AddComponentInternal(struct avdl_node *o, int size, void (*constructor)(void *)) {
	struct avdl_component *c = malloc(size);
	dd_da_push(&o->components, c);
	constructor(c);
	// each component has a reference to the node they are attached to
	c->node = o;
	return c;
}

static void avdl_node_printInternal(struct avdl_node *o, int tabs) {

	// Print tabs (if any)
	for (int i = 0; i < tabs; i++) {
		printf("\t");
	}

	if (tabs == 0) {
		printf("avdl_node:\n");
		printf("*** ");
	}
	else {
		printf("* ");
	}

	struct dd_vec3 *pos = avdl_transform_GetPosition(&o->localTransform);
	printf("node_name | position %f %f %f", pos->x, pos->y, pos->z);
	printf("\n");

	// Print children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_get(&o->children, i);
		avdl_node_printInternal(child, tabs+1);
	}
}

void avdl_node_print(struct avdl_node *o) {
	avdl_node_printInternal(o, 0);
}

