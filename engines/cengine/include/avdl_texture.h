#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_graphics.h"
#include "dd_dynamic_array.h"
#include "avdl_assetManager.h"

enum AVDL_IMAGETYPE {
	AVDL_IMAGETYPE_PNG,
	AVDL_IMAGETYPE_BMP,
};

struct avdl_texture {
	#ifdef AVDL_DIRECT3D11
	avdl_texture_id tex;
	int *pixelsb;
	#else
	GLuint tex;
	GLubyte *pixelsb;
	struct dd_dynamic_array subpixels;
	#endif
	int width, height;
	float *pixels;
	const char *assetName;
	int assetType;
	int openglContextId;

	int pixelFormat;

	// new image
	struct avdl_assetManager_texture *texture;
	int dirtyTexture;

	void (*bind)(struct avdl_texture *o);
	void (*bindIndex)(struct avdl_texture *o, int index);
	void (*bindIndexArray)(struct avdl_texture *o, int index, int arraySize, struct avdl_texture *array[]);
	void (*unbind)(struct avdl_texture *o);
	void (*unbindIndex)(struct avdl_texture *o, int index);
	void (*unbindIndexArray)(struct avdl_texture *o, int index);
	void (*clean)(struct avdl_texture *o);
	void (*set)(struct avdl_texture *o, const char *filename, int type);

	void (*addSubpixels)(struct avdl_texture *o, void *pixels, int pixel_format, int x, int y, int w, int h);

	int (*isLoaded)(struct avdl_texture *o);
};

void avdl_texture_create(struct avdl_texture *o);

#ifdef AVDL_DIRECT3D11
void avdl_texture_load_bmp(struct avdl_texture *img, const char *filename);
#elif defined(WIN32) || defined(_WIN32)
void avdl_texture_load_bmp(struct avdl_texture *img, const wchar_t *filename);
#else
void avdl_texture_load_bmp(struct avdl_texture *img, const char *filename);
#endif
int avdl_texture_load_png(struct avdl_texture *img, const char *filename);
int avdl_texture_load_FromAsset(struct avdl_texture *img, struct avdl_assetManager_texture *t);

void avdl_texture_bind(struct avdl_texture *o);
void avdl_texture_bindIndex(struct avdl_texture *o, int index);
void avdl_texture_bindIndexArray(struct avdl_texture *o, int index, int arraySize, struct avdl_texture *array[]);
void avdl_texture_unbind(struct avdl_texture *o);
void avdl_texture_unbindIndex(struct avdl_texture *o, int index);
void avdl_texture_unbindIndexArray(struct avdl_texture *o, int index);
void avdl_texture_set(struct avdl_texture *o, const char *filename, int type);
void avdl_texture_setLocal(struct avdl_texture *o, const char *filename, int type);

void avdl_texture_clean(struct avdl_texture *o);

void avdl_texture_addSubpixels(struct avdl_texture *o, void *pixels, int pixel_format, int x, int y, int w, int h);

int avdl_texture_isLoaded(struct avdl_texture *o);

void avdl_texture_cleanNonGpuData(struct avdl_texture *o);

#ifdef __cplusplus
}
#endif

#endif
