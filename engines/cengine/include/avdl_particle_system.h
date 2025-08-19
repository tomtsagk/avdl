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

#include "avdl_mesh.h"
#include "avdl_vec3.h"

/*
 * the maximum number of particles that can
 * be active at a time, for now
 */
#define PARTICLES_TOTAL 50

#ifdef __cplusplus
extern "C" {
#endif

/*
 * each individual particle's data
 */
struct avdl_particle {

	// particles location and mesh
	struct dd_matrix matrix;
	struct avdl_mesh *mesh;

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
	struct avdl_mesh *particleMesh;
	float particleLife;
	struct avdl_vec3 particlePosition;
	struct avdl_vec3 particlePositionFuzz;
	struct avdl_vec3 particleRotation;
	struct avdl_vec3 particleRotationFuzz;
	struct avdl_vec3 particleScale;
	struct avdl_vec3 particleScaleFuzz;

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

}; // particle system

void avdl_particle_system_create(struct avdl_particle_system *);
void avdl_particle_system_clean(struct avdl_particle_system *);

void avdl_particle_system_assignAsset(struct avdl_particle_system *, struct avdl_mesh *);
void avdl_particle_system_update(struct avdl_particle_system *, float dt);
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

#ifdef __cplusplus
}
#endif

#endif
