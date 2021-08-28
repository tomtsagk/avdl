#ifndef AVDL_PARTICLE_SYSTEM_H
#define AVDL_PARTICLE_SYSTEM_H

/*
 * Particle System
 *
 * Generates particles every so often, that
 * draw themselves based on the given mesh.
 *
 * They have an initial position, rotation and
 * scale, with some fuzziness, and also
 * the ability to give functions that control
 * their animated transformations.
 */

#include "dd_mesh.h"
#include "dd_vec3.h"

/*
 * the maximum number of particles that can
 * be active at a time, for now
 */
#define PARTICLES_TOTAL 50

/*
 * each individual particle's data
 */
struct avdl_particle {

	// particles location and mesh
	struct dd_matrix matrix;
	struct dd_mesh *mesh;

	// current life of particle
	float life;

};

/*
 * particle system that spawns particles every
 * so often.
 */
struct avdl_particle_system {

	// particles array
	struct avdl_particle particles[PARTICLES_TOTAL];
	int particlesCount;
	int particlesStart;

	// artificial limit
	int particlesTotal;

	// how often particles appear
	float delayMax;
	float delayCurrent;

	// values to be given to each new particle
	struct dd_mesh *particleMesh;
	float particleLife;
	struct dd_vec3 particlePosition;
	struct dd_vec3 particlePositionFuzz;
	struct dd_vec3 particleRotation;
	struct dd_vec3 particleRotationFuzz;
	struct dd_vec3 particleScale;
	struct dd_vec3 particleScaleFuzz;

	/*
	 * functions to allow variable values
	 */
	float (*particlePositionXFunction)(float);
	float (*particlePositionYFunction)(float);
	float (*particlePositionZFunction)(float);
	float (*particleScaleFunction)(float);

	/*
	 * basic functions
	 */
	void (*clean)(struct avdl_particle_system *);
	void (*update)(struct avdl_particle_system *);
	void (*draw)(struct avdl_particle_system *);

	/*
	 * setters for particle data
	 */
	void (*assignAsset)(struct avdl_particle_system *, struct dd_mesh *);
	void (*setDelay)(struct avdl_particle_system *, float);
	void (*setParticleLife)(struct avdl_particle_system *, float);
	void (*setParticlePositionXFunc)(struct avdl_particle_system *, float (*)(float));
	void (*setParticlePositionYFunc)(struct avdl_particle_system *, float (*)(float));
	void (*setParticlePositionZFunc)(struct avdl_particle_system *, float (*)(float));
	void (*setParticleScaleFunc)(struct avdl_particle_system *, float (*)(float));
	void (*setParticlesTotal)(struct avdl_particle_system *, int);

	// initial value setters
	void (*setParticlePosition)(struct avdl_particle_system *, float, float, float);
	void (*setParticlePositionFuzz)(struct avdl_particle_system *, float, float, float);
	void (*setParticleRotation)(struct avdl_particle_system *, float, float, float);
	void (*setParticleRotationFuzz)(struct avdl_particle_system *, float, float, float);
	void (*setParticleScale)(struct avdl_particle_system *, float, float, float);
	void (*setParticleScaleFuzz)(struct avdl_particle_system *, float, float, float);

}; // particle system

void avdl_particle_system_create(struct avdl_particle_system *);
void avdl_particle_system_clean(struct avdl_particle_system *);

void avdl_particle_system_assignAsset(struct avdl_particle_system *, struct dd_mesh *);
void avdl_particle_system_update(struct avdl_particle_system *);
void avdl_particle_system_draw(struct avdl_particle_system *);

void avdl_particle_system_setDelay(struct avdl_particle_system *, float newDelay);
void avdl_particle_system_setParticleLife(struct avdl_particle_system *, float pLife);
void avdl_particle_system_setParticlePositionXFunc(struct avdl_particle_system *o, float (*func)(float));
void avdl_particle_system_setParticlePositionYFunc(struct avdl_particle_system *o, float (*func)(float));
void avdl_particle_system_setParticlePositionZFunc(struct avdl_particle_system *o, float (*func)(float));
void avdl_particle_system_setParticleScaleFunc(struct avdl_particle_system *o, float (*func)(float));

void avdl_particle_system_setParticlePosition    (struct avdl_particle_system *o, float x, float y, float z);
void avdl_particle_system_setParticlePositionFuzz(struct avdl_particle_system *o, float x, float y, float z);
void avdl_particle_system_setParticleRotation    (struct avdl_particle_system *o, float x, float y, float z);
void avdl_particle_system_setParticleRotationFuzz(struct avdl_particle_system *o, float x, float y, float z);
void avdl_particle_system_setParticleScale       (struct avdl_particle_system *o, float x, float y, float z);
void avdl_particle_system_setParticleScaleFuzz   (struct avdl_particle_system *o, float x, float y, float z);

void avdl_particle_system_setParticlesTotal(struct avdl_particle_system *o, int);

#endif
