#include "avdl_rigidbody.h"
#include "dd_matrix.h"

void avdl_rigidbody_create(struct avdl_rigidbody *o) {
	o->collider = 0;
	o->matrixMultiply = avdl_rigidbody_matrixMultiply;
	o->mass = 0;
	o->setPositionf = avdl_rigidbody_setPositionf;
	o->setMass = avdl_rigidbody_setMass;
	o->setCollider = avdl_rigidbody_setCollider;

	dd_vec3_setf(&o->position, 0, 0, 0);
	dd_vec3_setf(&o->velocity, 0, 0, 0);
}

void avdl_rigidbody_clean(struct avdl_rigidbody *o) {
}

void avdl_rigidbody_matrixMultiply(struct avdl_rigidbody *o) {
	dd_translatef(o->position.x, o->position.y, o->position.z);
}

void avdl_rigidbody_setPositionf(struct avdl_rigidbody *o, float x, float y, float z) {
	dd_vec3_setf(&o->position, x, y, z);
}

void avdl_rigidbody_setMass(struct avdl_rigidbody *o, float m) {
	o->mass = m;
	if (m != 0) {
		o->mass_inv = 1/m;
	}
	else {
		o->mass_inv = 0;
	}
}

void avdl_rigidbody_setCollider(struct avdl_rigidbody *o, struct avdl_collider *collider) {
	o->collider = collider;
}
