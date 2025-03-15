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

	// new image
	struct avdl_assetManager_texture *texture;
	int dirtyTexture;

	// updating parts of the image
	struct dd_dynamic_array subpixels;

	void (*clean)(struct avdl_texture *o);
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
int avdl_texture_UnLoad(struct avdl_texture *o);

void avdl_texture_cleanNonGpuData(struct avdl_texture *o);

int avdl_texture_CreateTexture(struct avdl_texture *o, int width, int height, int pixelFormat);

int avdl_texture_GetWidth(struct avdl_texture *o);
int avdl_texture_GetHeight(struct avdl_texture *o);
int avdl_texture_GetPixelFormat(struct avdl_texture *o);
void *avdl_texture_GetPixels(struct avdl_texture *o);

#ifdef __cplusplus
}
#endif

#endif
