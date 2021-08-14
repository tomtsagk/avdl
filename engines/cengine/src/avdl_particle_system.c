#include "avdl_particle_system.h"
#include "dd_math.h"

/*
 * particle init data
 */

void avdl_particle_system_create(struct avdl_particle_system *o) {
	o->particleMesh = 0;
	o->delayMax = o->delayCurrent = 0;
	o->particlesCount = 0;
	o->particlesStart = 0;

	o->particleLife = 0;
	dd_vec3_create(&o->particleSpeed);

	o->assignAsset = avdl_particle_system_assignAsset;
	o->update = avdl_particle_system_update;
	o->draw = avdl_particle_system_draw;

	o->setDelay = avdl_particle_system_setDelay;
	o->setParticleLife = avdl_particle_system_setParticleLife;
	o->setParticleSpeed = avdl_particle_system_setParticleSpeed;
}

void avdl_particle_system_clean(struct avdl_particle_system *o) {
}

void avdl_particle_system_assignAsset(struct avdl_particle_system *o, struct dd_mesh *mesh) {
	o->particleMesh = mesh;
}

void avdl_particle_system_update(struct avdl_particle_system *o) {

	// update each particle
	for (int i = 0; i < o->particlesCount; i++) {
		int index = (o->particlesStart +i) %PARTICLES_TOTAL;
		struct dd_vec3 gravity;
		dd_vec3_create(&gravity);
		dd_vec3_set(&gravity, 0, -0.01, 0);
		dd_vec3_add(&o->particles[index].speed, &gravity);
		dd_matrix_translate(&o->particles[index].matrix,
			o->particles[index].speed.x,
			o->particles[index].speed.y,
			o->particles[index].speed.z
		);

		o->particles[index].life -= 1;
		if (o->particles[index].life <= 0) {
			o->particlesStart = (o->particlesStart +1) %PARTICLES_TOTAL;
			o->particlesCount--;
		}
	}

	// decide if a new particle needs to appear
	o->delayCurrent -= 1;
	if (o->delayCurrent <= 0 && o->particlesCount < PARTICLES_TOTAL) {
		int index = (o->particlesStart +o->particlesCount) %PARTICLES_TOTAL;
		o->delayCurrent = o->delayMax;
		o->particles[index].mesh = o->particleMesh;
		dd_matrix_identity(&o->particles[index].matrix);
		dd_vec3_create(&o->particles[index].speed);
		//dd_vec3_set(&o->particles[index].speed, -0.5 +dd_math_rand(100)/100.0, 0.5, 0);
		dd_vec3_set(&o->particles[index].speed,
			dd_vec3_getX(&o->particleSpeed),
			dd_vec3_getY(&o->particleSpeed),
			dd_vec3_getZ(&o->particleSpeed)
		);
		o->particles[index].life = o->particleLife;
		o->particlesCount++;
	}
}

void avdl_particle_system_draw(struct avdl_particle_system *o) {

	for (int i = 0; i < o->particlesCount; i++) {
		int index = (o->particlesStart +i) %PARTICLES_TOTAL;
		if (o->particles[index].mesh) {
			dd_matrix_push();
			dd_multMatrixf(&o->particles[index].matrix);
			o->particles[index].mesh->draw(o->particles[index].mesh);
			dd_matrix_pop();
		}
	}
}

void avdl_particle_system_setDelay(struct avdl_particle_system *o, float newDelay) {
	o->delayMax = newDelay;
}

void avdl_particle_system_setParticleLife(struct avdl_particle_system *o, float pLife) {
	o->particleLife = pLife;
}

void avdl_particle_system_setParticleSpeed(struct avdl_particle_system *o, float x, float y, float z) {
	o->particleSpeed.set(&o->particleSpeed, x, y, z);
}
