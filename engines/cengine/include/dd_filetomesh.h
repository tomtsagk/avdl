#ifndef DD_FILETOMESH_H
#define DD_FILETOMESH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Needs mesh, to store data into */
#include "dd_mesh.h"
#include "dd_matrix.h"
#include "dd_vec3.h"
#include "dd_vec4.h"

/* Files supported */
#define DD_PLY 0
#define DD_OBJ 1
#define AVDL_GLTF 2

/* Settings for different things */

/* Mirroring - can be piped together for multiple mirroring */
#define DD_FILETOMESH_SETTINGS_MIRROR_X 1
#define DD_FILETOMESH_SETTINGS_MIRROR_Y 2
#define DD_FILETOMESH_SETTINGS_MIRROR_Z 4

/* Vertex attributes to parse */
#define DD_FILETOMESH_SETTINGS_POSITION 8
#define DD_FILETOMESH_SETTINGS_COLOUR 16
#define DD_FILETOMESH_SETTINGS_TEX_COORD 32

#include "dd_dynamic_array.h"

struct dd_keyframe_vec3 {
	struct dd_vec3 value;
	float time;
};

struct dd_keyframe_vec4 {
	struct dd_vec4 value;
	float time;
};

struct dd_animated_bone {
	struct dd_dynamic_array keyframes_position;
	struct dd_dynamic_array keyframes_rotation;
	struct dd_dynamic_array keyframes_scale;
};

struct dd_animation {
	char *name;
	struct dd_animated_bone *animatedBones;
	int animatedBonesCount;
};

/* a mesh with all the asked data loaded
 */
struct dd_loaded_mesh {
	int vcount;
	float *v;
	float *c;
	float *t;
	float *n;
	float *tan;
	float *bitan;
	int *boneIds;
	float *weights;

	// animations
	int boneCount;
	struct dd_matrix *inverseBindMatrices;
	struct dd_animation *animations;
	int animationsCount;
	int rootIndex;
	int **children_indices;
	int *children_indices_count;
	struct dd_matrix rootMatrix;
};

#ifdef AVDL_DIRECT3D11
int dd_filetomesh(struct dd_loaded_mesh *m, const char *path, int settings, int file_type);
/* Creates a loaded mesh from a file */
#elif defined(_WIN32) || defined(WIN32)
int dd_filetomesh(struct dd_loaded_mesh *m, const wchar_t *path, int settings, int file_type);
#else
int dd_filetomesh(struct dd_loaded_mesh *m, const char *path, int settings, int file_type);
#endif

#ifdef __cplusplus
}
#endif

#endif /* DD_FILETOMESH_H */
