#ifndef AVDL_SKELETON_H
#define AVDL_SKELETON_H

#include "dd_matrix.h"
#include "dd_filetomesh.h"

struct avdl_animated_bone {
	struct dd_vec3 *positions;
	struct dd_vec4 *rotations;
	struct dd_vec3 *scales;
	float *positions_time;
	float *rotations_time;
	float *scales_time;
	int keyframe_count_positions;
	int keyframe_count_rotations;
	int keyframe_count_scales;
};

struct avdl_animation {
	char *name;
	struct avdl_animated_bone *animatedBones;
	int animatedBonesCount;
};

#define AVDL_SKELETON_NUMBER_OF_BONES 100

struct avdl_skeleton {

	// current animation
	float currentTime;
	int currentAnimationIndex;
	struct avdl_animation *animations;
	int animations_count;

	int boneCount;
	struct dd_matrix *inverseBindMatrices;
	struct dd_matrix finalMatrices[AVDL_SKELETON_NUMBER_OF_BONES];
	int rootIndex;

	int **children_indices;
	int *children_indices_count;

	struct dd_matrix rootMatrix;

	int isActive;

	const char *queuedAnimation;
};

void avdl_skeleton_create(struct avdl_skeleton *o);
void avdl_skeleton_clean(struct avdl_skeleton *o);
void avdl_skeleton_update(struct avdl_skeleton *o, float dt);
void avdl_skeleton_PlayAnimation(struct avdl_skeleton *o, const char *animName);
struct dd_matrix *avdl_skeleton_GetFinalMatrices(struct avdl_skeleton *o);

void avdl_skeleton_SetAnimations(struct avdl_skeleton *o, int animationsCount, struct dd_animation *animations);
int avdl_skeleton_IsActive(struct avdl_skeleton *o);
void avdl_skeleton_Activate(struct avdl_skeleton *o);

#endif
