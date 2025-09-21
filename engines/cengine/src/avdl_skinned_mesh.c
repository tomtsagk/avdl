#include "avdl_skinned_mesh.h"
#include "shared/avdl_log.h"
#include <string.h>
#include <stdlib.h>

#include "avdl_mesh.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

static void clean_boneIds(struct avdl_skinned_mesh *m) {
	if (m->boneIds && m->dirtyBoneIds) {
		free(m->boneIds);
		m->dirtyBoneIds = 0;
	}
	m->boneIds = 0;
}
static void clean_weights(struct avdl_skinned_mesh *m) {
	if (m->weights && m->dirtyWeights) {
		free(m->weights);
		m->dirtyWeights = 0;
	}
	m->weights = 0;
}

// constructor
void avdl_skinned_mesh_create(struct avdl_skinned_mesh *m) {

	avdl_mesh_create(&m->parent);

	// skeleton
	m->boneIds = 0;
	m->dirtyBoneIds = 0;
	m->weights = 0;
	m->dirtyWeights = 0;

	avdl_skeleton_create(&m->skeleton);
}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void avdl_skinned_mesh_clean(struct avdl_skinned_mesh *m) {
	avdl_mesh_clean(m);
	clean_boneIds(m);
	clean_weights(m);
	avdl_skeleton_clean(&m->skeleton);
}

extern struct dd_matrix matPerspective;
extern struct dd_matrix matView;
extern struct dd_matrix matModel[];
extern int matModel_index;

/* draw the mesh itself
 */
void avdl_skinned_mesh_draw(struct avdl_skinned_mesh *m) {
	if (!m->parent.data) {
		return;
	}
	// skeleton exists if there are bones, inverse bind matrices and at least one animation
	if (m->parent.data->boneCount > 0 && m->parent.data->inverseBindMatrices && m->parent.data->animationsCount > 0 && m->skeleton.boneCount == 0) {
		avdl_skeleton_Activate(&m->skeleton);

		// skeleton data
		m->skeleton.boneCount = m->parent.data->boneCount;
		m->skeleton.inverseBindMatrices = m->parent.data->inverseBindMatrices;
		m->skeleton.rootIndex = m->parent.data->rootIndex;
		m->skeleton.children_indices = m->parent.data->children_indices;
		m->skeleton.children_indices_count = m->parent.data->children_indices_count;
		dd_matrix_copy(&m->skeleton.rootMatrix, &m->parent.data->rootMatrix);

		avdl_skeleton_SetAnimations(&m->skeleton, m->parent.data->animationsCount, m->parent.data->animations);
	}
	if (avdl_skeleton_IsActive(&m->skeleton)) {
		int boneMatricesId = avdl_graphics_GetUniformLocation(currentProgram, "avdl_finalBonesMatrices");
		if (boneMatricesId >= 0) {
			avdl_graphics_SetUniformMatrix4fMultiple(boneMatricesId, AVDL_SKELETON_NUMBER_OF_BONES, avdl_skeleton_GetFinalMatrices(&m->skeleton));
		}
	}
	avdl_mesh_draw(m);
}

void avdl_skinned_mesh_update(struct avdl_skinned_mesh *o, float dt) {
	avdl_skeleton_update(&o->skeleton, dt);
}

void avdl_skinned_mesh_PlayAnimation(struct avdl_skinned_mesh *o, const char *animName) {
	avdl_skeleton_PlayAnimation(&o->skeleton, animName, 0);
}

void avdl_skinned_mesh_PlayAnimationInstant(struct avdl_skinned_mesh *o, const char *animName) {
	avdl_skeleton_PlayAnimation(&o->skeleton, animName, 1);
}

void avdl_skinned_mesh_PrintAnimations(struct avdl_skinned_mesh *o) {
	if (!avdl_skeleton_IsActive(&o->skeleton)) return;
	for (int i = 0; i < o->skeleton.animations_count; i++) {
		avdl_log(o->skeleton.animations[i].name);
	}
}

void avdl_skinned_mesh_SetOnAnimationDone(struct avdl_skinned_mesh *o, void (*func)(void *ctx), void *context) {
	o->skeleton.SetOnAnimationDone(&o->skeleton, func, context);
}
