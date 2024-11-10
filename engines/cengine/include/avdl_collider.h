#ifndef AVDL_COLLIDER_H
#define AVDL_COLLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_vec3.h"
#include "dd_mesh.h"
#include "avdl_node.h"

#define AVDL_COLLIDER_TYPE_POINT 0
#define AVDL_COLLIDER_TYPE_AABB 1
#define AVDL_COLLIDER_TYPE_OBB 2
#define AVDL_COLLIDER_TYPE_SPHERE 3

struct avdl_collider {
	int type;
};

void avdl_collider_create(struct avdl_collider *o);
void avdl_collider_clean(struct avdl_collider *o);

int avdl_collider_collision(struct avdl_collider *o1, struct dd_matrix *m1, struct dd_matrix *nm1, struct avdl_collider *o2, struct dd_matrix *m2, struct dd_matrix *nm2);
int avdl_collider_collisionNode(struct avdl_collider *o1, struct avdl_node *n1, struct avdl_collider *o2, struct avdl_node *n2);

#ifdef __cplusplus
}
#endif

#endif
