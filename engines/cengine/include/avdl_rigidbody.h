#ifndef AVDL_RIGIDBODY_H
#define AVDL_RIGIDBODY_H

#include "avdl_collider.h"

struct avdl_rigidbody {

	struct avdl_collider *collider;

	struct dd_vec3 position;
	struct dd_vec3 velocity;
	float mass;
	float mass_inv;

	void (*matrixMultiply)(struct avdl_rigidbody *);
	void (*setPositionf)(struct avdl_rigidbody *, float, float, float);
	void (*setMass)(struct avdl_rigidbody *, float);
	void (*setCollider)(struct avdl_rigidbody *, struct avdl_collider *);
};

void avdl_rigidbody_create(struct avdl_rigidbody *);
void avdl_rigidbody_clean(struct avdl_rigidbody *);

void avdl_rigidbody_setPositionf(struct avdl_rigidbody *, float, float, float);
void avdl_rigidbody_setMass(struct avdl_rigidbody *, float);

void avdl_rigidbody_setCollider(struct avdl_rigidbody *, struct avdl_collider *);

void avdl_rigidbody_matrixMultiply(struct avdl_rigidbody *);

#endif
