#ifndef AVDL_NODE_H
#define AVDL_NODE_H

#include "avdl_transform.h"
#include "dd_matrix.h"
#include "dd_dynamic_array.h"
#include "avdl_string.h"

#define AVDL_NODE_NAME_LENGTH 100

struct avdl_node {

	// parent node (optional)
	struct avdl_node *parent;

	// node name
	struct avdl_string name;

	// local transform
	struct avdl_transform localTransform;

	// matrix of all parents + local
	struct dd_matrix globalMatrix;
	struct dd_matrix globalNormalMatrix;
	struct dd_matrix globalInverseMatrix;
	struct dd_matrix globalNormalInverseMatrix;

	// component
	struct dd_dynamic_array components;

	// children nodes
	struct dd_dynamic_array children;

	void (*clean)(struct avdl_node *);

};

void avdl_node_create(struct avdl_node *o);
void avdl_node_clean(struct avdl_node *o);
void avdl_node_print(struct avdl_node *o);

struct avdl_transform *avdl_node_GetLocalTransform(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalNormalMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalInverseMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalNormalInverseMatrix(struct avdl_node *o);

struct avdl_node *avdl_node_AddChild(struct avdl_node *o);
int avdl_node_RemoveChild(struct avdl_node *o, struct avdl_node *child);
struct avdl_component *avdl_node_AddComponentInternal(struct avdl_node *o, int size, void (*constructor)(void *));
#define avdl_node_AddComponent(x, y) avdl_node_AddComponentInternal(x, sizeof(struct y), y ## _create);

void avdl_node_SetName(struct avdl_node *o, const char *name);
const char *avdl_node_GetName(struct avdl_node *o);
int avdl_node_GetChildrenCount(struct avdl_node *o);
struct avdl_node *avdl_node_GetChild(struct avdl_node *o, int index);
struct avdl_node *avdl_node_GetParent(struct avdl_node *o);

void avdl_node_AddComponentsToArray_Internal(struct avdl_node *o, struct dd_dynamic_array *array, void (*fnc)(struct avdl_component *));
#define avdl_node_AddComponentsToArray(o, array, component) avdl_node_AddComponentsToArray_Internal(o, array, component ## _clean)
int avdl_node_GetComponentCount(struct avdl_node *o);
struct avdl_component *avdl_node_GetComponent_Internal(struct avdl_node *o, void (*fnc)(struct avdl_component *));
#define avdl_node_GetComponent(o, component) avdl_node_GetComponent_Internal(o, component ## _clean)

int avdl_node_NodeToJson(struct avdl_node *o, char *filename);
int avdl_node_JsonToNode(char *filename, struct avdl_node *o);
int avdl_node_Copy(struct avdl_node *o, struct avdl_node *target);
struct avdl_node *avdl_node_Duplicate(struct avdl_node *o, struct avdl_node *newParent);

#endif
