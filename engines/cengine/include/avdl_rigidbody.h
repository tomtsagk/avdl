#ifndef AVDL_RIGIDBODY_H
#define AVDL_RIGIDBODY_H

#include "avdl_collider.h"
#include "dd_vec3.h"
#include "dd_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_rigidbody {

	struct avdl_collider *collider;

	// movement
	struct dd_vec3 position;
	struct dd_vec3 velocity;

	// rotation
	struct dd_matrix rotation;
	struct dd_matrix angularVelocity;
	struct dd_vec3 angularVelocityVec3;

	float mass;
	float mass_inv;
	float restitution;

	int has_just_collided;

	void (*matrixMultiply)(struct avdl_rigidbody *);
	void (*setPositionf)(struct avdl_rigidbody *, float, float, float);
	float (*getPositionX)(struct avdl_rigidbody *);
	float (*getPositionY)(struct avdl_rigidbody *);
	float (*getPositionZ)(struct avdl_rigidbody *);
	void (*setMass)(struct avdl_rigidbody *, float);
	void (*setRestitution)(struct avdl_rigidbody *, float);
	void (*setCollider)(struct avdl_rigidbody *, struct avdl_collider *);

	void (*addVelocityf)(struct avdl_rigidbody *, float, float, float);
	void (*addAngularVelocityf)(struct avdl_rigidbody *, float, float, float);
	int (*hasJustCollided)(struct avdl_rigidbody *);

	void (*reset)(struct avdl_rigidbody *);
};

void avdl_rigidbody_create(struct avdl_rigidbody *);
void avdl_rigidbody_clean(struct avdl_rigidbody *);

void avdl_rigidbody_setPositionf(struct avdl_rigidbody *, float, float, float);
void avdl_rigidbody_setMass(struct avdl_rigidbody *, float);
void avdl_rigidbody_setRestitution(struct avdl_rigidbody *, float);

void avdl_rigidbody_setCollider(struct avdl_rigidbody *, struct avdl_collider *);

void avdl_rigidbody_matrixMultiply(struct avdl_rigidbody *);

void avdl_rigidbody_addVelocityf(struct avdl_rigidbody *, float, float, float);
void avdl_rigidbody_addAngularVelocityf(struct avdl_rigidbody *, float, float, float);
int avdl_rigidbody_hasJustCollided(struct avdl_rigidbody *);

float avdl_rigidbody_getPositionX(struct avdl_rigidbody *);
float avdl_rigidbody_getPositionY(struct avdl_rigidbody *);
float avdl_rigidbody_getPositionZ(struct avdl_rigidbody *);

void avdl_rigidbody_reset(struct avdl_rigidbody *);

#ifdef __cplusplus
}
#endif

#endif
