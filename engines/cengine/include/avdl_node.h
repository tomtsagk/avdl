#ifndef AVDL_NODE_H
#define AVDL_NODE_H

#include "avdl_transform.h"
#include "dd_matrix.h"
#include "dd_dynamic_array.h"

#define AVDL_NODE_NAME_LENGTH 100

struct avdl_node {

	// parent node (optional)
	struct avdl_node *parent;

	// node name
	char name[AVDL_NODE_NAME_LENGTH];

	// local transform
	struct avdl_transform localTransform;

	// matrix of all parents + local
	struct dd_matrix globalMatrix;
	struct dd_matrix globalNormalMatrix;
	struct dd_matrix globalInverseMatrix;

	// component
	struct dd_dynamic_array components;

	// children nodes
	struct dd_dynamic_array children;

	void (*create)(struct avdl_node *);
	void (*clean)(struct avdl_node *);

	struct avdl_transform *(*GetLocalTransform)(struct avdl_node *);
	struct avdl_transform *(*GetGlobalMatrix)(struct avdl_node *);
	struct avdl_transform *(*GetGlobalNormalMatrix)(struct avdl_node *);
	struct avdl_transform *(*GetGlobalInverseMatrix)(struct avdl_node *);

	struct avdl_node *(*AddChild)(struct avdl_node *);
	void (*SetName)(struct avdl_node *, char *name);
	char *(*GetName)(struct avdl_node *);

	int (*GetChildrenCount)(struct avdl_node *);
	int (*GetChild)(struct avdl_node *, int index);

	void (*AddComponentsToArray)(struct avdl_node *, struct dd_dynamic_array *array, int component_type);

};

void avdl_node_create(struct avdl_node *o);
void avdl_node_clean(struct avdl_node *o);
void avdl_node_print(struct avdl_node *o);

struct avdl_transform *avdl_node_GetLocalTransform(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalNormalMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalInverseMatrix(struct avdl_node *o);

struct avdl_node *avdl_node_AddChild(struct avdl_node *o);
struct avdl_component *avdl_node_AddComponentInternal(struct avdl_node *o, int size, void (*constructor)(void *));
#define avdl_node_AddComponent(x, y) avdl_node_AddComponentInternal(x, sizeof(struct y), y ## _create);

void avdl_node_SetName(struct avdl_node *o, char *name);
char *avdl_node_GetName(struct avdl_node *o);
int avdl_node_GetChildrenCount(struct avdl_node *o);
struct avdl_node *avdl_node_GetChild(struct avdl_node *o, int index);

void avdl_node_AddComponentsToArray(struct avdl_node *o, struct dd_dynamic_array *array, int component_type);
struct avdl_component *avdl_node_GetComponent(struct avdl_node *o, int component_type);

int avdl_node_NodeToJson(struct avdl_node *o, char *filename);
int avdl_node_JsonToNode(char *filename, struct avdl_node *o);

#endif
