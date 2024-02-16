#ifndef AVDL_SKYBOX
#define AVDL_SKYBOX

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_image.h"
#include "avdl_mesh.h"
#include "avdl_shaders.h"

struct avdl_skybox {

	// skybox mesh - cube
	struct avdl_mesh mesh;

	// shader program
	struct avdl_program program;

	// metadata
	int graphics_contextid;
	char *assetName;
	int assetType;

	// textures
	struct dd_image img[6];

	// avdl id for skybox
	avdl_texture_id tex;

	void (*clean)(struct avdl_skybox *o);
	void (*set)(struct avdl_skybox *o, const char *assets[]);
	void (*bind)(struct avdl_skybox *o);
	void (*unbind)(struct avdl_skybox *o);
	void (*draw)(struct avdl_skybox *o);
};

void avdl_skybox_create(struct avdl_skybox *o);
void avdl_skybox_clean(struct avdl_skybox *o);

void avdl_skybox_set(struct avdl_skybox *o, const char *assetname[]);

void avdl_skybox_draw(struct avdl_skybox *o);
void avdl_skybox_bind(struct avdl_skybox *o);
void avdl_skybox_unbind(struct avdl_skybox *o);

#ifdef __cplusplus
}
#endif

#endif
