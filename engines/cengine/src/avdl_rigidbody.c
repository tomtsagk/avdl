#include "avdl_rigidbody.h"
#include "dd_matrix.h"

void avdl_rigidbody_create(struct avdl_rigidbody *o) {
	o->collider = 0;
	o->has_just_collided = 0;
	o->has_just_collided_old = 0;

	avdl_rigidbody_setMass(o, 1);
	avdl_rigidbody_setRestitution(o, 1);

	avdl_vec3_Setf(&o->position, 0, 0, 0);
	avdl_vec3_Setf(&o->velocity, 0, 0, 0);

	dd_matrix_identity(&o->rotation);
	dd_matrix_identity(&o->angularVelocity);
	avdl_vec3_Setf(&o->angularVelocityVec3, 0, 0, 0);
}

void avdl_rigidbody_matrixMultiply(struct avdl_rigidbody *o) {
	dd_translatef(o->position.x, o->position.y, o->position.z);
	dd_multMatrixf(&o->rotation);
}

void avdl_rigidbody_setPositionf(struct avdl_rigidbody *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->position, x, y, z);
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

void avdl_rigidbody_setRestitution(struct avdl_rigidbody *o, float r) {
	o->restitution = r;
}

void avdl_rigidbody_setCollider(struct avdl_rigidbody *o, struct avdl_collider *collider) {
	o->collider = collider;
}

void avdl_rigidbody_addVelocityf(struct avdl_rigidbody *o, float x, float y, float z) {
	avdl_vec3_Addf(&o->velocity, x, y, z);
}

void avdl_rigidbody_setVelocityf(struct avdl_rigidbody *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->velocity, x, y, z);
}

void avdl_rigidbody_addAngularVelocityf(struct avdl_rigidbody *o, float x, float y, float z) {
	struct dd_matrix m;
	dd_matrix_identity(&m);
	dd_matrix_rotate(&m, x, 1, 0, 0);
	dd_matrix_rotate(&m, y, 0, 1, 0);
	dd_matrix_rotate(&m, z, 0, 0, 1);
	//dd_matrix_mult(&m, &o->rotation);
	//dd_matrix_copy(&o->rotation, &m);
	dd_matrix_mult(&m, &o->angularVelocity);
	dd_matrix_copy(&o->angularVelocity, &m);

	avdl_vec3_Addf(&o->angularVelocityVec3, x, y, z);
}

void avdl_rigidbody_setAngularVelocityf(struct avdl_rigidbody *o, float x, float y, float z) {
	struct dd_matrix m;
	dd_matrix_identity(&m);
	dd_matrix_rotate(&m, x, 1, 0, 0);
	dd_matrix_rotate(&m, y, 0, 1, 0);
	dd_matrix_rotate(&m, z, 0, 0, 1);
	dd_matrix_identity(&o->angularVelocity);
	dd_matrix_mult(&m, &o->angularVelocity);
	dd_matrix_copy(&o->angularVelocity, &m);

	avdl_vec3_Setf(&o->angularVelocityVec3, x, y, z);
}

float avdl_rigidbody_getPositionX(struct avdl_rigidbody *o) {
	return o->position.x;
}

float avdl_rigidbody_getPositionY(struct avdl_rigidbody *o) {
	return o->position.y;
}

float avdl_rigidbody_getPositionZ(struct avdl_rigidbody *o) {
	return o->position.z;
}

void avdl_rigidbody_reset(struct avdl_rigidbody *o) {
	avdl_vec3_Setf(&o->velocity, 0, 0, 0);
	avdl_vec3_Setf(&o->angularVelocityVec3, 0, 0, 0);
	dd_matrix_identity(&o->rotation);
	dd_matrix_identity(&o->angularVelocity);
}

int avdl_rigidbody_hasJustCollided(struct avdl_rigidbody *o) {
	return o->has_just_collided;
}
