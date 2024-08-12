#ifndef AVDL_SKINNED_MESH_H
#define AVDL_SKINNED_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_assetManager.h"
#include "avdl_skeleton.h"
#include "avdl_mesh.h"

struct avdl_skinned_mesh {
	struct avdl_mesh parent;

	// skeletons
	int *boneIds;
	int dirtyBoneIds;
	float *weights;
	int dirtyWeights;

	struct avdl_skeleton skeleton;

	// animation
	void (*update)(struct avdl_skinned_mesh *, float dt);
	void (*PlayAnimation)(struct avdl_skinned_mesh *, const char *animName);
	void (*SetOnAnimationDone)(struct avdl_skinned_mesh *, void (*func)(void *ctx), void *context);

};

// constructor
void avdl_skinned_mesh_create(struct avdl_skinned_mesh *);
void avdl_skinned_mesh_clean(struct avdl_skinned_mesh *m);
void avdl_skinned_mesh_draw(struct avdl_skinned_mesh *m);

// animations
void avdl_skinned_mesh_update(struct avdl_skinned_mesh *m, float dt);
void avdl_skinned_mesh_PlayAnimation(struct avdl_skinned_mesh *m, const char *name);
void avdl_skinned_mesh_PrintAnimations(struct avdl_skinned_mesh *m);

void avdl_skinned_mesh_LoadFromLoadedMesh(struct avdl_skinned_mesh *o, struct dd_loaded_mesh *loadedMesh);
void avdl_skinned_mesh_SetOnAnimationDone(struct avdl_skinned_mesh *o, void (*func)(void *ctx), void *context);

#ifdef __cplusplus
}
#endif

#endif /* SKINNED_MESH_H */
