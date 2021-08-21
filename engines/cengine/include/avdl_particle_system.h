#ifndef AVDL_PARTICLE_SYSTEM_H
#define AVDL_PARTICLE_SYSTEM_H

#include "dd_mesh.h"
#include "dd_vec4.h"
#include "dd_vec3.h"

/*
 * the maximum number of particles that can
 * be active at a time
 */
#define PARTICLES_TOTAL 10

/*
 * each individual particle's data
 */
struct avdl_particle {

	// particles location and mesh
	struct dd_matrix matrix;
	struct dd_mesh *mesh;

	// current speed of particle
	struct dd_vec3 speed;

	// current life of particle
	float life;
};

/*
 * particle system that spawns particles every
 * so often.
 */
struct avdl_particle_system {

	// active particles
	struct avdl_particle particles[PARTICLES_TOTAL];
	int particlesCount;
	int particlesStart;

	// how often particles appear
	float delayMax;
	float delayCurrent;

	// values to be given to each new particle
	struct dd_mesh *particleMesh;
	float particleLife;
	struct dd_vec3 particleSpeed;

	void (*clean)(struct avdl_particle_system *);
	void (*update)(struct avdl_particle_system *);
	void (*draw)(struct avdl_particle_system *);

	void (*assignAsset)(struct avdl_particle_system *, struct dd_mesh *);
	void (*setDelay)(struct avdl_particle_system *, float);
	void (*setParticleLife)(struct avdl_particle_system *, float);
	void (*setParticleSpeed)(struct avdl_particle_system *, float, float, float);
};

void avdl_particle_system_create(struct avdl_particle_system *);
void avdl_particle_system_clean(struct avdl_particle_system *);

void avdl_particle_system_assignAsset(struct avdl_particle_system *, struct dd_mesh *);
void avdl_particle_system_update(struct avdl_particle_system *);
void avdl_particle_system_draw(struct avdl_particle_system *);

void avdl_particle_system_setDelay(struct avdl_particle_system *, float newDelay);
void avdl_particle_system_setParticleLife(struct avdl_particle_system *, float pLife);
void avdl_particle_system_setParticleSpeed(struct avdl_particle_system *, float x, float y, float z);

#endif
