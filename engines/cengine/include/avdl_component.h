#ifndef AVDL_COMPONENT_H
#define AVDL_COMPONENT_H

enum AVDL_COMPONENTS {
	avdl_component_collider_aabb_enum,
	avdl_component_CUSTOM,
};

struct avdl_component {

	enum AVDL_COMPONENTS type;
	struct avdl_node *node;

	void (*create)(struct avdl_component *);
	void (*clean)(struct avdl_component *);

	void (*SetType)(struct avdl_component *, int type);

	struct avdl_node *(*GetNode)(struct avdl_component *);
};

void avdl_component_create(struct avdl_component *o);
void avdl_component_clean(struct avdl_component *o);

struct avdl_node *avdl_component_GetNode(struct avdl_component *o);
void avdl_component_SetType(struct avdl_component *o, int type);

#endif
