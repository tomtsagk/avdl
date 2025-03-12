#include "avdl_particle_system.h"
#include "dd_math.h"
#include "avdl_log.h"

/*
 * Particle System Initialisation
 *
 * Everything should be disabled. At a minimum,
 * the "particleMesh" and the "life" should be set
 * to a mingful value, for particles to appear.
 */
void avdl_particle_system_create(struct avdl_particle_system *o) {
	o->particleMesh = 0;
	o->delayMax = o->delayCurrent = 0;
	o->particlesCount = 0;
	o->particlesStart = 0;
	o->particlesTotal = PARTICLES_TOTAL;

	o->particleLife = 0;

	o->particlePositionXFunction = 0;
	o->particlePositionYFunction = 0;
	o->particlePositionZFunction = 0;

	o->particleScaleFunction = 0;

	// init vectors
	avdl_vec3_create(&o->particlePosition);
	avdl_vec3_create(&o->particlePositionFuzz);
	avdl_vec3_Setf(&o->particlePosition, 0, 0, 0);
	avdl_vec3_Setf(&o->particlePositionFuzz, 0, 0, 0);
	avdl_vec3_create(&o->particleRotation);
	avdl_vec3_create(&o->particleRotationFuzz);
	avdl_vec3_Setf(&o->particleRotation, 0, 0, 0);
	avdl_vec3_Setf(&o->particleRotationFuzz, 0, 0, 0);
	avdl_vec3_create(&o->particleScale);
	avdl_vec3_create(&o->particleScaleFuzz);
	avdl_vec3_Setf(&o->particleScale, 1, 1, 1);
	avdl_vec3_Setf(&o->particleScaleFuzz, 0, 0, 0);

	o->assignAsset = avdl_particle_system_assignAsset;
	o->update = avdl_particle_system_update;
	o->draw = avdl_particle_system_draw;

	o->setDelay = avdl_particle_system_setDelay;
	o->setParticleLife = avdl_particle_system_setParticleLife;
	o->setParticlePositionXFunc = avdl_particle_system_setParticlePositionXFunc;
	o->setParticlePositionYFunc = avdl_particle_system_setParticlePositionYFunc;
	o->setParticlePositionZFunc = avdl_particle_system_setParticlePositionZFunc;
	o->setParticleScaleFunc = avdl_particle_system_setParticleScaleFunc;

	// init values setters functions
	o->setParticlePosition = avdl_particle_system_setParticlePosition;
	o->setParticlePositionFuzz = avdl_particle_system_setParticlePositionFuzz;
	o->setParticleRotation = avdl_particle_system_setParticleRotation;
	o->setParticleRotationFuzz = avdl_particle_system_setParticleRotationFuzz;
	o->setParticleScale = avdl_particle_system_setParticleScale;
	o->setParticleScaleFuzz = avdl_particle_system_setParticleScaleFuzz;
	o->setParticlesTotal = avdl_particle_system_setParticlesTotal;
}

void avdl_particle_system_clean(struct avdl_particle_system *o) {
}

void avdl_particle_system_assignAsset(struct avdl_particle_system *o, struct avdl_mesh *mesh) {
	o->particleMesh = mesh;
}

void avdl_particle_system_update(struct avdl_particle_system *o, float dt) {

	// update each particle
	for (int i = 0; i < o->particlesCount; i++) {
		int index = (o->particlesStart +i) %PARTICLES_TOTAL;

		// update remaining life
		o->particles[index].life -= dt;
		if (o->particles[index].life <= 0) {
			o->particlesStart = (o->particlesStart +1) %PARTICLES_TOTAL;
			o->particlesCount--;
		}
	}

	// decide if a new particle needs to appear
	o->delayCurrent -= dt;
	if (o->delayCurrent <= 0 && o->particlesCount < o->particlesTotal) {
		int index = (o->particlesStart +o->particlesCount) %PARTICLES_TOTAL;
		o->delayCurrent = o->delayMax;
		o->particles[index].mesh = o->particleMesh;

		// particle's transformation
		dd_matrix_identity(&o->particles[index].matrix);

		// position
		dd_matrix_translate(&o->particles[index].matrix,
			avdl_vec3_X(&o->particlePosition)
				+dd_math_randf(avdl_vec3_X(&o->particlePositionFuzz))
				-avdl_vec3_X(&o->particlePositionFuzz)/2,
			avdl_vec3_Y(&o->particlePosition)
				+dd_math_randf(avdl_vec3_Y(&o->particlePositionFuzz))
				-avdl_vec3_Y(&o->particlePositionFuzz)/2,
			avdl_vec3_Z(&o->particlePosition)
				+dd_math_randf(avdl_vec3_Z(&o->particlePositionFuzz))
				-avdl_vec3_Z(&o->particlePositionFuzz)/2
		);

		// rotation
		dd_matrix_rotate(&o->particles[index].matrix,
			avdl_vec3_X(&o->particleRotation)
				+ dd_math_randf(avdl_vec3_X(&o->particleRotationFuzz)),
			1,
			0,
			0
		);
		dd_matrix_rotate(&o->particles[index].matrix,
			avdl_vec3_Y(&o->particleRotation)
				+ dd_math_randf(avdl_vec3_Y(&o->particleRotationFuzz)),
			0,
			1,
			0
		);
		dd_matrix_rotate(&o->particles[index].matrix,
			avdl_vec3_Z(&o->particleRotation)
				+ dd_math_randf(avdl_vec3_Z(&o->particleRotationFuzz)),
			0,
			0,
			1
		);

		// scale
		dd_matrix_scale(&o->particles[index].matrix,
			avdl_vec3_X(&o->particleScale)
				+dd_math_randf(avdl_vec3_X(&o->particleScaleFuzz))
				-avdl_vec3_X(&o->particleScaleFuzz)/2,
			avdl_vec3_Y(&o->particleScale)
				+dd_math_randf(avdl_vec3_Y(&o->particleScaleFuzz))
				-avdl_vec3_Y(&o->particleScaleFuzz)/2,
			avdl_vec3_Z(&o->particleScale)
				+dd_math_randf(avdl_vec3_Z(&o->particleScaleFuzz))
				-avdl_vec3_Z(&o->particleScaleFuzz)/2
		);

		// life
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

			// calculate animated position
			float t = 1 -(o->particles[index].life /o->particleLife);
			float updateX = 0;
			if (o->particlePositionXFunction) {
				updateX = o->particlePositionXFunction(t);
			}

			float updateY = 0;
			if (o->particlePositionYFunction) {
				updateY = o->particlePositionYFunction(t);
			}

			float updateZ = 0;
			if (o->particlePositionZFunction) {
				updateZ = o->particlePositionZFunction(t);
			}

			dd_translatef(updateX, updateY, updateZ);

			if (o->particleScaleFunction) {
				float t = o->particleScaleFunction(1 -(o->particles[index].life /o->particleLife));
				dd_scalef(t, t, t);
			}
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

void avdl_particle_system_setParticlePositionXFunc(struct avdl_particle_system *o, float (*func)(float)) {
	o->particlePositionXFunction = func;
}

void avdl_particle_system_setParticlePositionYFunc(struct avdl_particle_system *o, float (*func)(float)) {
	o->particlePositionYFunction = func;
}

void avdl_particle_system_setParticlePositionZFunc(struct avdl_particle_system *o, float (*func)(float)) {
	o->particlePositionZFunction = func;
}

void avdl_particle_system_setParticleScaleFunc(struct avdl_particle_system *o, float (*func)(float)) {
	o->particleScaleFunction = func;
}

void avdl_particle_system_setParticlePosition(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particlePosition, x, y, z);
}

void avdl_particle_system_setParticlePositionFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particlePositionFuzz, x, y, z);
}

void avdl_particle_system_setParticleRotation(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particleRotation, x, y, z);
}

void avdl_particle_system_setParticleRotationFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particleRotationFuzz, x, y, z);
}

void avdl_particle_system_setParticleScale(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particleScale, x, y, z);
}

void avdl_particle_system_setParticleScaleFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->particleScaleFuzz, x, y, z);
}

void avdl_particle_system_setParticlesTotal(struct avdl_particle_system *o, int particlesTotal) {
	if (particlesTotal > PARTICLES_TOTAL) {
		avdl_log("avdl error: cannot have %d particles, limiting to %d",
			particlesTotal, PARTICLES_TOTAL
		);
		particlesTotal = PARTICLES_TOTAL;
	}
	o->particlesTotal = particlesTotal;
}
