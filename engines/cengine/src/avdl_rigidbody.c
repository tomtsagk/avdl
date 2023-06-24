#include "avdl_rigidbody.h"
#include "dd_matrix.h"

void avdl_rigidbody_create(struct avdl_rigidbody *o) {
	o->collider = 0;

	o->matrixMultiply = avdl_rigidbody_matrixMultiply;
	o->setPositionf = avdl_rigidbody_setPositionf;
	o->setMass = avdl_rigidbody_setMass;
	o->setRestitution = avdl_rigidbody_setRestitution;
	o->setCollider = avdl_rigidbody_setCollider;
	o->addVelocityf = avdl_rigidbody_addVelocityf;
	o->getPositionX = avdl_rigidbody_getPositionX;
	o->getPositionY = avdl_rigidbody_getPositionY;
	o->getPositionZ = avdl_rigidbody_getPositionZ;
	o->reset = avdl_rigidbody_reset;

	o->setMass(o, 1);
	o->setRestitution(o, 1);

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

void avdl_rigidbody_setRestitution(struct avdl_rigidbody *o, float r) {
	o->restitution = r;
}

void avdl_rigidbody_setCollider(struct avdl_rigidbody *o, struct avdl_collider *collider) {
	o->collider = collider;
}

void avdl_rigidbody_addVelocityf(struct avdl_rigidbody *o, float x, float y, float z) {
	dd_vec3_addf(&o->velocity, x, y, z);
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
	dd_vec3_setf(&o->velocity, 0, 0, 0);
}
