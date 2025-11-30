#ifndef AVDL_COLLIDER_H
#define AVDL_COLLIDER_H

//#include <ode/ode.h>

#include "avdl_vec3.h"
#include "avdl_vec4.h"
#include "avdl_node.h"
#include "avdl_mesh.h"
#include "shared/avdl_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AVDL_COLLIDER_TYPE_POINT 0
#define AVDL_COLLIDER_TYPE_AABB 1
#define AVDL_COLLIDER_TYPE_OBB 2
#define AVDL_COLLIDER_TYPE_SPHERE 3
#define AVDL_COLLIDER_TYPE_TERRRAIN 4

struct avdl_collider_collision {
	struct avdl_vec4 overlap;
	struct avdl_vec3 normal1;
	struct avdl_vec3 normal2;
};

void avdl_collider_collision_create(struct avdl_collider_collision *o);
void avdl_collider_collision_clean(struct avdl_collider_collision *o);
struct avdl_vec4 *avdl_collider_collision_GetOverlap(struct avdl_collider_collision *o);
struct avdl_vec3 *avdl_collider_collision_GetNormal1(struct avdl_collider_collision *o);
struct avdl_vec3 *avdl_collider_collision_GetNormal2(struct avdl_collider_collision *o);

struct avdl_collider {
	int type;
	//dGeomID geom;
	void *geom;
};

void avdl_collider_create(struct avdl_collider *o);
void avdl_collider_clean(struct avdl_collider *o);

int avdl_collider_collision(struct avdl_collider *o1, struct dd_matrix *m1, struct dd_matrix *nm1, struct avdl_collider *o2, struct dd_matrix *m2, struct dd_matrix *nm2);
int avdl_collider_collisionNode(struct avdl_collider *o1, struct avdl_node *n1, struct avdl_collider *o2, struct avdl_node *n2, struct avdl_collider_collision *collision);

int avdl_collider_DrawDebug(struct avdl_collider *o, struct avdl_node *n);
int avdl_collider_DrawDebugNode(struct avdl_collider *o, struct avdl_node *n);

void avdl_collider_init();
void avdl_collider_deinit();

#ifdef __cplusplus
}
#endif

#endif
