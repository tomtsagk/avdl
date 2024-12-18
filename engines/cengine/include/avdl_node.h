#ifndef AVDL_NODE_H
#define AVDL_NODE_H

#include "avdl_transform.h"
#include "dd_matrix.h"
#include "dd_dynamic_array.h"

struct avdl_node {

	// parent node (optional)
	struct avdl_node *parent;

	// local transform
	struct avdl_transform localTransform;

	// matrix of all parents + local
	struct dd_matrix globalMatrix;
	struct dd_matrix globalNormalMatrix;

	// component
	struct dd_dynamic_array components;

	// children nodes
	struct dd_dynamic_array children;

	void (*create)(struct avdl_node *);
	void (*clean)(struct avdl_node *);

	struct avdl_transform *(*GetLocalTransform)(struct avdl_node *);
	struct avdl_transform *(*GetGlobalMatrix)(struct avdl_node *);
	struct avdl_transform *(*GetGlobalNormalMatrix)(struct avdl_node *);

	struct avdl_node *(*AddChild)(struct avdl_node *);

};

void avdl_node_create(struct avdl_node *o);
void avdl_node_clean(struct avdl_node *o);
void avdl_node_print(struct avdl_node *o);

struct avdl_transform *avdl_node_GetLocalTransform(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalMatrix(struct avdl_node *o);
struct dd_matrix *avdl_node_GetGlobalNormalMatrix(struct avdl_node *o);

struct avdl_node *avdl_node_AddChild(struct avdl_node *o);
struct avdl_component *avdl_node_AddComponentInternal(struct avdl_node *o, int size, void (*constructor)(void *));
#define avdl_node_AddComponent(x, y) avdl_node_AddComponentInternal(x, sizeof(struct y), y ## _create);

#endif
