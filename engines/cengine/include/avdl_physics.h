#ifndef AVDL_PHYSICS_H
#define AVDL_PHYSICS_H

#include "avdl_collider.h"
#include "avdl_rigidbody.h"
#include "dd_vec3.h"

struct avdl_physics {
	struct avdl_rigidbody *object[10];
	int object_count;
	struct dd_vec3 constant_force;

	void (*addObject)(struct avdl_physics *, struct avdl_rigidbody *);
	void (*update)(struct avdl_physics *);

	void (*addConstantForcef)(struct avdl_physics *, float, float, float);
	void (*clearConstantForce)(struct avdl_physics *);
};

void avdl_physics_create(struct avdl_physics *o);
void avdl_physics_update(struct avdl_physics *o);
void avdl_physics_clean(struct avdl_physics *o);
void avdl_physics_addObject(struct avdl_physics *o, struct avdl_rigidbody *obj);

void avdl_physics_addConstantForcef(struct avdl_physics *o, float x, float y, float z);
void avdl_physics_clearConstantForce(struct avdl_physics *o);

#endif
