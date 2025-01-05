#ifndef AVDL_COMPONENT_H
#define AVDL_COMPONENT_H

enum AVDL_COMPONENTS {
	AVDL_COMPONENT_CUSTOM_ENUM,

	AVDL_COMPONENT_MESH_ENUM = 200,
	AVDL_COMPONENT_CUSTOM_EDITOR_ENUM,
	AVDL_COMPONENT_INAVLID_ENUM,
};

struct avdl_component {

	enum AVDL_COMPONENTS type;
	struct avdl_node *node;

	void (*clean)(struct avdl_component *);

	void (*after_create)(struct avdl_component *);

	void (*SetType)(struct avdl_component *, int type);
	int (*GetType)(struct avdl_component *);

	struct avdl_node *(*GetNode)(struct avdl_component *);
};

void avdl_component_create(struct avdl_component *o);
void avdl_component_clean(struct avdl_component *o);
void avdl_component_after_create(struct avdl_component *o);

struct avdl_node *avdl_component_GetNode(struct avdl_component *o);
void avdl_component_SetType(struct avdl_component *o, int type);
int avdl_component_GetType(struct avdl_component *o);

#endif
