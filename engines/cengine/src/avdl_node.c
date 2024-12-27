#include "avdl_node.h"
#include "dd_log.h"
#include "avdl_component.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void avdl_node_create(struct avdl_node *o) {
	o->parent = 0;

	o->name[0] = '\0';

	o->GetLocalTransform = avdl_node_GetLocalTransform;
	o->GetGlobalMatrix = avdl_node_GetGlobalMatrix;
	o->AddChild = avdl_node_AddChild;
	o->SetName = avdl_node_SetName;
	o->AddComponentsToArray = avdl_node_AddComponentsToArray;

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
	dd_da_push(&o->components, &c);
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
	printf("%s | position %f %f %f\n", o->name[0] != '\0' ? o->name : "no_node_name", pos->x, pos->y, pos->z);

	// Print components
	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		printf("	component %d\n", i);
	}

	// Print children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_get(&o->children, i);
		avdl_node_printInternal(child, tabs+1);
	}
}

void avdl_node_print(struct avdl_node *o) {
	avdl_node_printInternal(o, 0);
}

void avdl_node_SetName(struct avdl_node *o, char *name) {

	if (strlen(name) >= AVDL_NODE_NAME_LENGTH) {
		dd_log("cannot copy node name, too big: %s", name);
		return;
	}

	//dd_log("set name to : %s", name);
	strncpy(o->name, name, AVDL_NODE_NAME_LENGTH-1);
	o->name[AVDL_NODE_NAME_LENGTH-1] = '\0';
}

void avdl_node_AddComponentsToArray(struct avdl_node *o, struct dd_dynamic_array *array, int component_type) {

	// Print components
	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		struct avdl_component *c = *((struct avdl_component **) dd_da_get(&o->components, i));

		if (c->type == component_type) {
			dd_da_push(array, &c);
		}
	}

	// Print children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_get(&o->children, i);
		avdl_node_AddComponentsToArray(child, array, component_type);
	}
}

void avdl_node_NodeToJson(struct avdl_node *o, char *filename) {
}

void avdl_node_JsonToNode(char *filename, struct avdl_node *o) {
}
