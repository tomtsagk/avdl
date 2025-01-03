#include "avdl_skeleton.h"
#include "string.h"
#include "avdl_log.h"
#include "dd_vec3.h"
#include "dd_math.h"
#include "dd_matrix.h"

#define MIX_TIME 0.2

void avdl_skeleton_create(struct avdl_skeleton *o) {
	o->currentTime = 0;
	o->currentAnimationIndex = -1;
	o->animations = 0;
	o->animations_count = 0;

	o->boneCount = 0;
	o->inverseBindMatrices = 0;
	o->rootIndex = -1;
	o->children_indices = 0;
	o->children_indices_count = 0;

	o->isActive = 0;

	o->queuedAnimation = 0;
	o->queuedAnimationInstant = 0;

	o->OnAnimationDone = 0;
	o->SetOnAnimationDone = avdl_skeleton_SetOnAnimationDone;
	o->OnAnimationContext = 0;

	o->mixBones = 0;
}

void avdl_skeleton_clean(struct avdl_skeleton *o) {
	if (!o->isActive) return;
	o->isActive = 0;

	for (int i = 0; i < o->animations_count; i++) {
		struct avdl_animation *anim = &o->animations[i];
		for (int j = 0; j < o->boneCount; j++) {
			struct avdl_animated_bone *animBone = &anim->animatedBones[j];
			free(animBone->positions);
			free(animBone->positions_time);
			free(animBone->rotations);
			free(animBone->rotations_time);
		}
		free(anim->animatedBones);
		free(anim->name);
	}
	for (int i = 0; i < o->boneCount; i++) {
		free(o->children_indices[i]);
	}
	free(o->children_indices);
	free(o->children_indices_count);
	free(o->animations);
	free(o->inverseBindMatrices);
	if (o->mixBones) {
		free(o->mixBones);
		o->mixBones = 0;
	}
}

#define QUATERNION_EPS (1e-4)
static void quaternion_slerp(struct dd_vec4 *q1, struct dd_vec4 *q2, float t, struct dd_vec4 *output) {

	float dot =
		q1->cell[0] * q2->cell[0] +
		q1->cell[1] * q2->cell[1] +
		q1->cell[2] * q2->cell[2] +
		q1->cell[3] * q2->cell[3];
	struct dd_vec4 q2temp;
	q2temp.cell[0] = q2->cell[0];
	q2temp.cell[1] = q2->cell[1];
	q2temp.cell[2] = q2->cell[2];
	q2temp.cell[3] = q2->cell[3];
	if (dot < 0.0) {
		q2temp.cell[0] *= -1;
		q2temp.cell[1] *= -1;
		q2temp.cell[2] *= -1;
		q2temp.cell[3] *= -1;
	}

	// Calculate angle between them.
	double cosHalfTheta =
		q1->cell[3] * q2temp.cell[3] + q1->cell[0] * q2temp.cell[0] + q1->cell[1] * q2temp.cell[1] + q1->cell[2] * q2temp.cell[2];

	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if ( fabs( cosHalfTheta ) >= 1.0 ) {
		output->cell[0] = q1->cell[0];
		output->cell[1] = q1->cell[1];
		output->cell[2] = q1->cell[2];
		output->cell[3] = q1->cell[3];
		return;
	}

	// Calculate temporary values.
	double halfTheta    = acos( cosHalfTheta );
	double sinHalfTheta = sqrt( 1.0 - cosHalfTheta * cosHalfTheta );

	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if ( fabs( sinHalfTheta ) < 0.001 ) { // fabs is floating point absolute
		output->cell[3] = ( q1->cell[3] * 0.5 + q2temp.cell[3] * 0.5 );
		output->cell[0] = ( q1->cell[0] * 0.5 + q2temp.cell[0] * 0.5 );
		output->cell[1] = ( q1->cell[1] * 0.5 + q2temp.cell[1] * 0.5 );
		output->cell[2] = ( q1->cell[2] * 0.5 + q2temp.cell[2] * 0.5 );
		return;
	}

	double ratioA = sin( ( 1 - t ) * halfTheta ) / sinHalfTheta;
	double ratioB = sin( t * halfTheta ) / sinHalfTheta;
	// calculate Quaternion.
	output->cell[3] = ( q1->cell[3] * ratioA + q2temp.cell[3] * ratioB );
	output->cell[0] = ( q1->cell[0] * ratioA + q2temp.cell[0] * ratioB );
	output->cell[1] = ( q1->cell[1] * ratioA + q2temp.cell[1] * ratioB );
	output->cell[2] = ( q1->cell[2] * ratioA + q2temp.cell[2] * ratioB );
}

static void interpolate_position(struct avdl_skeleton *o, int index, struct dd_matrix *parentTransform) {
	int i = index;
	dd_matrix_identity(&o->finalMatrices[i]);

	// no animation playing, T pose all bones
	if (o->currentAnimationIndex < 0) {
		for (int j = 0; j < o->children_indices_count[i]; j++) {
			interpolate_position(o, o->children_indices[i][j], 0);
		}
		return;
	}

	struct avdl_animated_bone *b = &o->animations[o->currentAnimationIndex].animatedBones[i];

	int keyframe_index = -1;
	// find current keyframe index
	for (int j = 0; j < b->keyframe_count_positions; j++) {
		//avdl_log("keyframe: %d", j);
		if (o->currentTime < b->positions_time[j]) {
			// before first keyframe
			if (j == 0) {
				keyframe_index = -2;
				break;
			}

			// keyframe found
			keyframe_index = j-1;
			break;
		}
	}

	struct dd_vec3 *pos;
	struct dd_vec3 *pos2;
	float animationLength;
	// mixing
	if (o->currentTime < 0) {
		pos = &o->mixBones[i].position;
		pos2 = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[0];
		animationLength = (o->currentTime +MIX_TIME) /MIX_TIME;
	}
	else
	// before first keyframe
	if (keyframe_index == -1) {
		pos = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[0];
		pos2 = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[0];
		animationLength = 0.5;
	}
	else
	// after last keyframe
	if (keyframe_index == -2) {
		pos = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[b->keyframe_count_positions-1];
		pos2 = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[b->keyframe_count_positions-1];
		animationLength = 0.5;
	}
	// normal keyframe
	else {
		pos = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[keyframe_index];
		pos2 = &o->animations[o->currentAnimationIndex].animatedBones[i].positions[keyframe_index+1];
		animationLength = (o->currentTime -b->positions_time[keyframe_index]) /(b->positions_time[keyframe_index+1] -b->positions_time[keyframe_index]);
		animationLength = dd_math_max(animationLength, 0);
	}

	int keyframe_rotation_index = -1;
	// find current keyframe index
	for (int j = 0; j < b->keyframe_count_rotations; j++) {
		//avdl_log("keyframe: %d", j);
		if (o->currentTime < b->rotations_time[j]) {
			// before first keyframe
			if (j == 0) {
				keyframe_rotation_index = -2;
				break;
			}
			keyframe_rotation_index = j-1;
			break;
		}
	}
	struct dd_vec4 *rot;
	struct dd_vec4 *rot2;
	float animationRotationLength;
	// mixing
	if (o->currentTime < 0) {
		rot = &o->mixBones[i].rotation;
		rot2 = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[0];
		animationRotationLength = (o->currentTime +MIX_TIME) /MIX_TIME;
	}
	else
	// before first keyframe
	if (keyframe_rotation_index == -1) {
		rot = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[0];
		rot2 = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[0];
		animationRotationLength = 0;
	}
	else
	// after last keyframe
	if (keyframe_rotation_index == -2) {
		rot = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[b->keyframe_count_rotations-1];
		rot2 = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[b->keyframe_count_rotations-1];
		animationRotationLength = 0;
	}
	else {
		rot = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[keyframe_rotation_index];
		rot2 = &o->animations[o->currentAnimationIndex].animatedBones[i].rotations[keyframe_rotation_index+1];
		animationRotationLength = (o->currentTime -b->rotations_time[keyframe_rotation_index]) /(b->rotations_time[keyframe_rotation_index+1] -b->rotations_time[keyframe_rotation_index]);
		animationRotationLength = dd_math_max(animationRotationLength, 0);
	}

	// interpolate rotations
	struct dd_vec4 slerped;
	quaternion_slerp(rot, rot2, animationRotationLength, &slerped);
	struct dd_matrix rotMat;
	dd_matrix_create(&rotMat);
	dd_matrix_identity(&rotMat);
	dd_matrix_quaternion_to_rotation_matrix(&slerped, &rotMat);

	struct dd_matrix globalTransform;
	dd_matrix_identity(&globalTransform);

	// root bone gets its parent transform attached to it
	if (index == o->rootIndex) {
		dd_matrix_mult(&globalTransform, &o->rootMatrix);
	}

	if (parentTransform) {
		dd_matrix_mult(&globalTransform, parentTransform);
	}

	// save mix bones for mixing, only when not mixing
	if (o->currentTime >= 0) {
		dd_vec3_setf(&o->mixBones[i].position,
			pos->x +(pos2->x -pos->x) *animationLength,
			pos->y +(pos2->y -pos->y) *animationLength,
			pos->z +(pos2->z -pos->z) *animationLength
		);
		dd_vec4_set(&o->mixBones[i].rotation,
			slerped.cell[0],
			slerped.cell[1],
			slerped.cell[2],
			slerped.cell[3]
		);
	}

	// apply animated transforms
	dd_matrix_translate(&globalTransform,
		pos->x +(pos2->x -pos->x) *animationLength,
		pos->y +(pos2->y -pos->y) *animationLength,
		pos->z +(pos2->z -pos->z) *animationLength
	);
	dd_matrix_mult(&globalTransform, &rotMat);

	// assign to the final matrix the global transform + the inverse bind matrix
	dd_matrix_mult(&o->finalMatrices[i], &globalTransform);
	dd_matrix_mult(&o->finalMatrices[i], &o->inverseBindMatrices[i]);

	// recursively update all bone children
	for (int j = 0; j < o->children_indices_count[i]; j++) {
		interpolate_position(o, o->children_indices[i][j], &globalTransform);
	}
}

void avdl_skeleton_update(struct avdl_skeleton *o, float dt) {
	if (!o->isActive) return;
	if (o->queuedAnimation) {
		avdl_skeleton_PlayAnimation(o, o->queuedAnimation, o->queuedAnimationInstant);
		o->queuedAnimation = 0;
		o->queuedAnimationInstant = 0;
	}
	o->currentTime += 1.0 *dt;
	if (o->currentTime > o->animations[o->currentAnimationIndex].duration) {
		o->currentTime -= o->animations[o->currentAnimationIndex].duration;
		if (o->OnAnimationDone) {
			o->OnAnimationDone(o->OnAnimationContext);
		}
	}
	interpolate_position(o, o->rootIndex, 0);
}

void avdl_skeleton_PlayAnimation(struct avdl_skeleton *o, const char *animName, int instant) {
	if (!o->isActive) {
		o->queuedAnimation = animName;
		o->queuedAnimationInstant = instant;
		return;
	}
	int animIndex = -1;
	for (int i = 0; i < o->animations_count; i++) {
		if (strcmp(o->animations[i].name, animName) == 0) {
			animIndex = i;
		}
	}

	if (animIndex == -1) {
		avdl_log("avdl: skeleton: could not find animation with name '%s'", animName);
		return;
	}

	o->currentAnimationIndex = animIndex;
	if (instant) {
		o->currentTime = 0;
	}
	else {
		o->currentTime = -MIX_TIME;
	}
}

struct dd_matrix *avdl_skeleton_GetFinalMatrices(struct avdl_skeleton *o) {
	return o->finalMatrices;
}

void avdl_skeleton_SetAnimations(struct avdl_skeleton *o, int animationsCount, struct dd_animation *animations) {
	o->animations_count = animationsCount;
	o->animations = malloc(sizeof(struct avdl_animation) *animationsCount);
	o->mixBones = malloc(sizeof(struct avdl_mix_animated_bone) *o->boneCount);
	for (int i = 0; i < o->animations_count; i++) {
		struct avdl_animation *anim = &o->animations[i];
		anim->name = animations[i].name;
		anim->animatedBonesCount = animations[i].animatedBonesCount;
		anim->animatedBones = malloc(sizeof(struct avdl_animated_bone) *animations[i].animatedBonesCount);
		anim->duration = 0;
		for (int j = 0; j < o->boneCount; j++) {
			struct avdl_animated_bone *animBone = &anim->animatedBones[j];
			struct dd_animated_bone *animBoneSrc = &animations[i].animatedBones[j];
	
			// positions
			animBone->keyframe_count_positions = dd_da_count(&animBoneSrc->keyframes_position);
			animBone->positions = malloc(sizeof(struct dd_vec3) *animBone->keyframe_count_positions);
			animBone->positions_time = malloc(sizeof(float) *animBone->keyframe_count_positions);
			for (int k = 0; k < animBone->keyframe_count_positions; k++) {
				struct dd_vec3 *pos = &animBone->positions[k];
				struct dd_keyframe_vec3 *target = dd_da_get(&animBoneSrc->keyframes_position, k);
				animBone->positions_time[k] = target->time;
				pos->x = target->value.x;
				pos->y = target->value.y;
				pos->z = target->value.z;
				if (anim->duration < target->time) {
					anim->duration = target->time;
				}

				// init mix bone
				if (k == 0) {
					dd_vec3_setf(&o->mixBones[j].position, pos->x, pos->y, pos->z);
				}
			}
	
			// rotations
			animBone->keyframe_count_rotations = dd_da_count(&animBoneSrc->keyframes_rotation);
			animBone->rotations = malloc(sizeof(struct dd_vec4) *animBone->keyframe_count_rotations);
			animBone->rotations_time = malloc(sizeof(float) *animBone->keyframe_count_rotations);
			for (int k = 0; k < animBone->keyframe_count_rotations; k++) {
				struct dd_vec4 *rot = &animBone->rotations[k];
				struct dd_keyframe_vec4 *target = dd_da_get(&animBoneSrc->keyframes_rotation, k);
				animBone->rotations_time[k] = target->time;
				rot->cell[0] = target->value.cell[0];
				rot->cell[1] = target->value.cell[1];
				rot->cell[2] = target->value.cell[2];
				rot->cell[3] = target->value.cell[3];
				if (anim->duration < target->time) {
					anim->duration = target->time;
				}

				// init mix bone
				if (k == 0) {
					dd_vec4_set(&o->mixBones[j].rotation, rot->cell[0], rot->cell[1], rot->cell[2], rot->cell[3]);
				}
			}
			dd_da_free(&animBoneSrc->keyframes_position);
			dd_da_free(&animBoneSrc->keyframes_rotation);
		}
		free(animations[i].animatedBones);
	}
	free(animations);
}

int avdl_skeleton_IsActive(struct avdl_skeleton *o) {
	return o->isActive;
}

void avdl_skeleton_Activate(struct avdl_skeleton *o) {
	o->isActive = 1;
}

void avdl_skeleton_SetOnAnimationDone(struct avdl_skeleton *o, void (*func)(void *ctx), void *context) {
	o->OnAnimationDone = func;
	o->OnAnimationContext = context;
}
