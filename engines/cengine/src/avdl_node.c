#include "avdl_node.h"
#include "dd_log.h"
#include "avdl_component.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void avdl_node_create(struct avdl_node *o) {
	o->parent = 0;

	o->name[0] = '\0';

	o->GetLocalTransform = avdl_node_GetLocalTransform;
	o->GetGlobalMatrix = avdl_node_GetGlobalMatrix;
	o->AddChild = avdl_node_AddChild;
	o->SetName = avdl_node_SetName;
	o->GetName = avdl_node_GetName;
	o->AddComponentsToArray = avdl_node_AddComponentsToArray;

	o->GetChildrenCount = avdl_node_GetChildrenCount;
	o->GetChild = avdl_node_GetChild;

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

static void ReAssignComponentNodes(struct avdl_node *o) {
	for (int i = 0; i < dd_da_count(&o->components); i++) {
		struct avdl_component *c = dd_da_getDeref(&o->components, i);
		c->node = o;
	}

	for (int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_get(&o->children, i);
		ReAssignComponentNodes(child);
	}
}

struct avdl_node *avdl_node_AddChild(struct avdl_node *o) {
	dd_da_pushEmpty(&o->children);
	struct avdl_node *child = dd_da_get(&o->children, -1);
	avdl_node_create(child);
	child->parent = o;

	// components should get a fresh reference of their nodes, otherwise this introduces nasty bugs
	for (int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_get(&o->children, i);
		ReAssignComponentNodes(child);
	}

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
	if (dd_da_count(&o->components) > 0) {
		for (int i = 0; i < tabs+1; i++) {
			printf("\t");
		}
		for (int i = 0; i < dd_da_count(&o->components); i++) {
			printf("component: %d %x %x\n", i, dd_da_get(&o->components, i), dd_da_getDeref(&o->components, i));
		}
	}

	/*
	// Print components
	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		printf("	component %d\n", i);
	}
	*/

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

char *avdl_node_GetName(struct avdl_node *o) {
	if (o->name[0] == '\0') {
		return 0;
	}
	else {
		return o->name;
	}
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

int avdl_node_GetChildrenCount(struct avdl_node *o) {
	return dd_da_count(&o->children);
}

struct avdl_node *avdl_node_GetChild(struct avdl_node *o, int index) {
	if (index < 0 || index >= avdl_node_GetChildrenCount(o)) {
		dd_log("avdl_node_GetChild wrong index: %d", index);
		return 0;
	}
	return dd_da_get(&o->children, index);
}

static int NodeToJson_PrintTabs(int fd, int tabs) {
	char *content;
	for (int i = 0; i < tabs; i++) {
		content = "\t";
		write(fd, content, strlen(content));
	}
}

static int NodeToJson_PrintVec3(int fd, struct dd_vec3 *v) {
	char *content;
	char buffer[100];
	// x
	snprintf(buffer, 80, "%f", v->x);
	write(fd, buffer, strlen(buffer));

	content = ", ";
	write(fd, content, strlen(content));

	// y
	snprintf(buffer, 80, "%f", v->y);
	write(fd, buffer, strlen(buffer));

	content = ", ";
	write(fd, content, strlen(content));

	// z
	snprintf(buffer, 80, "%f", v->z);
	write(fd, buffer, strlen(buffer));

	return 0;
}

static int NodeToJson_PrintComponent(int fd, struct avdl_component *o, int tabs) {

	char *content;

	// start
	NodeToJson_PrintTabs(fd, tabs);
	content = "{\n";
	write(fd, content, strlen(content));

	NodeToJson_PrintTabs(fd, tabs);
	content = "\"name\": \"avdl_component_mesh\",\n";
	write(fd, content, strlen(content));

	// end
	NodeToJson_PrintTabs(fd, tabs);
	content = "}\n";
	write(fd, content, strlen(content));
}

static int NodeToJson_PrintNode(int fd, struct avdl_node *o, int tabs) {

	char *content;

	// start
	NodeToJson_PrintTabs(fd, tabs);
	content = "{\n";
	write(fd, content, strlen(content));

	// content
	if (avdl_node_GetName(o)) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"name\": \"";
		write(fd, content, strlen(content));
		content = avdl_node_GetName(o);
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));
	}
	else {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"name\": \"";
		write(fd, content, strlen(content));
		content = "NO_NAME";
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));
	}

	struct avdl_transform *t = avdl_node_GetLocalTransform(o);

	// position
	struct dd_vec3 *pos = avdl_transform_GetPosition(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"position\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, pos);
	content = " ],\n";
	write(fd, content, strlen(content));

	// rotation
	struct dd_vec3 *rot = avdl_transform_GetRotation(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"rotation\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, rot);
	content = " ],\n";
	write(fd, content, strlen(content));

	// scale
	struct dd_vec3 *scale = avdl_transform_GetScale(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"scale\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, scale);
	content = " ],\n";
	write(fd, content, strlen(content));

	// components
	if (dd_da_count(&o->components) > 0) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"components\": [\n";
		write(fd, content, strlen(content));
		for (int i = 0; i < dd_da_count(&o->components); i++) {
			struct avdl_component *component = dd_da_getDeref(&o->components, i);
			NodeToJson_PrintComponent(fd, component, tabs+1);
		}
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "]\n";
		write(fd, content, strlen(content));
	}

	// children
	if (dd_da_count(&o->children) > 0) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"children\": [\n";
		write(fd, content, strlen(content));
		for (int i = 0; i < dd_da_count(&o->children); i++) {
			struct avdl_node *child = dd_da_get(&o->children, i);
			NodeToJson_PrintNode(fd, child, tabs+1);
		}
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "]\n";
		write(fd, content, strlen(content));
	}

	// end
	NodeToJson_PrintTabs(fd, tabs);
	content = "}\n";
	write(fd, content, strlen(content));
}

int avdl_node_NodeToJson(struct avdl_node *o, char *filename) {
	dd_log("NodeToJson at %s", filename);

	remove(filename);

	int fd = open(filename, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		dd_log("NodeToJson: Unable to open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	int startingTabs = 0;
	NodeToJson_PrintNode(fd, o, startingTabs);

	close(fd);

	return 0;
}

int avdl_node_JsonToNode(char *filename, struct avdl_node *o) {
	return 0;
}
