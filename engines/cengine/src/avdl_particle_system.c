#include "avdl_particle_system.h"
#include "dd_math.h"
#include "dd_log.h"

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
	dd_vec3_create(&o->particlePosition);
	dd_vec3_create(&o->particlePositionFuzz);
	o->particlePosition.setf(&o->particlePosition, 0, 0, 0);
	o->particlePositionFuzz.setf(&o->particlePositionFuzz, 0, 0, 0);
	dd_vec3_create(&o->particleRotation);
	dd_vec3_create(&o->particleRotationFuzz);
	o->particleRotation.setf(&o->particleRotation, 0, 0, 0);
	o->particleRotationFuzz.setf(&o->particleRotationFuzz, 0, 0, 0);
	dd_vec3_create(&o->particleScale);
	dd_vec3_create(&o->particleScaleFuzz);
	o->particleScale.setf(&o->particleScale, 1, 1, 1);
	o->particleScaleFuzz.setf(&o->particleScaleFuzz, 0, 0, 0);

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

void avdl_particle_system_assignAsset(struct avdl_particle_system *o, struct dd_mesh *mesh) {
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
			o->particlePosition.getX(&o->particlePosition)
				+dd_math_randf(o->particlePositionFuzz.getX(&o->particlePositionFuzz))
				-o->particlePositionFuzz.getX(&o->particlePositionFuzz)/2,
			o->particlePosition.getY(&o->particlePosition)
				+dd_math_randf(o->particlePositionFuzz.getY(&o->particlePositionFuzz))
				-o->particlePositionFuzz.getY(&o->particlePositionFuzz)/2,
			o->particlePosition.getZ(&o->particlePosition)
				+dd_math_randf(o->particlePositionFuzz.getZ(&o->particlePositionFuzz))
				-o->particlePositionFuzz.getZ(&o->particlePositionFuzz)/2
		);

		// rotation
		dd_matrix_rotate(&o->particles[index].matrix,
			o->particleRotation.getX(&o->particleRotation)
				+ dd_math_randf(o->particleRotationFuzz.getX(&o->particleRotationFuzz)),
			1,
			0,
			0
		);
		dd_matrix_rotate(&o->particles[index].matrix,
			o->particleRotation.getY(&o->particleRotation)
				+ dd_math_randf(o->particleRotationFuzz.getY(&o->particleRotationFuzz)),
			0,
			1,
			0
		);
		dd_matrix_rotate(&o->particles[index].matrix,
			o->particleRotation.getZ(&o->particleRotation)
				+ dd_math_randf(o->particleRotationFuzz.getZ(&o->particleRotationFuzz)),
			0,
			0,
			1
		);

		// scale
		dd_matrix_scale(&o->particles[index].matrix,
			o->particleScale.getX(&o->particleScale)
				+dd_math_randf(o->particleScaleFuzz.getX(&o->particleScaleFuzz))
				-o->particleScaleFuzz.getX(&o->particleScaleFuzz)/2,
			o->particleScale.getY(&o->particleScale)
				+dd_math_randf(o->particleScaleFuzz.getY(&o->particleScaleFuzz))
				-o->particleScaleFuzz.getY(&o->particleScaleFuzz)/2,
			o->particleScale.getZ(&o->particleScale)
				+dd_math_randf(o->particleScaleFuzz.getZ(&o->particleScaleFuzz))
				-o->particleScaleFuzz.getZ(&o->particleScaleFuzz)/2
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
	o->particlePosition.setf(&o->particlePosition, x, y, z);
}

void avdl_particle_system_setParticlePositionFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	o->particlePositionFuzz.setf(&o->particlePositionFuzz, x, y, z);
}

void avdl_particle_system_setParticleRotation(struct avdl_particle_system *o, float x, float y, float z) {
	o->particleRotation.setf(&o->particleRotation, x, y, z);
}

void avdl_particle_system_setParticleRotationFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	o->particleRotationFuzz.setf(&o->particleRotationFuzz, x, y, z);
}

void avdl_particle_system_setParticleScale(struct avdl_particle_system *o, float x, float y, float z) {
	o->particleScale.setf(&o->particleScale, x, y, z);
}

void avdl_particle_system_setParticleScaleFuzz(struct avdl_particle_system *o, float x, float y, float z) {
	o->particleScaleFuzz.setf(&o->particleScaleFuzz, x, y, z);
}

void avdl_particle_system_setParticlesTotal(struct avdl_particle_system *o, int particlesTotal) {
	if (particlesTotal > PARTICLES_TOTAL) {
		dd_log("avdl error: cannot have %d particles, limiting to %d",
			particlesTotal, PARTICLES_TOTAL
		);
		particlesTotal = PARTICLES_TOTAL;
	}
	o->particlesTotal = particlesTotal;
}
