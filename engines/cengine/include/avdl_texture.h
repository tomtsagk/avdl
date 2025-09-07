#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_graphics.h"
#include "shared/avdl_dynamic_array.h"
#include "avdl_assetManager.h"

enum AVDL_IMAGETYPE {
	AVDL_IMAGETYPE_PNG,
	AVDL_IMAGETYPE_BMP,
};
struct avdl_texture_data;

struct avdl_texture {

	// new image
	struct avdl_texture_data *data;
	int dirtyTexture;

	// updating parts of the image
	struct avdl_dynamic_array subpixels;

	void (*clean)(struct avdl_texture *o);
};

void avdl_texture_create(struct avdl_texture *o);

void avdl_texture_bind(struct avdl_texture *o);
void avdl_texture_bindIndex(struct avdl_texture *o, int index);
void avdl_texture_unbind(struct avdl_texture *o);
void avdl_texture_unbindIndex(struct avdl_texture *o, int index);
void avdl_texture_set(struct avdl_texture *o, const char *filename, int type);
void avdl_texture_setLocal(struct avdl_texture *o, const char *filename, int type);

void avdl_texture_clean(struct avdl_texture *o);

void avdl_texture_addSubpixels(struct avdl_texture *o, void *pixels, int x, int y, int w, int h);

int avdl_texture_isLoaded(struct avdl_texture *o);

int avdl_texture_CreateTexture(struct avdl_texture *o, int width, int height, enum avdl_graphics_format_internal formatInternal, enum avdl_graphics_format format);

int avdl_texture_GetWidth(struct avdl_texture *o);
int avdl_texture_GetHeight(struct avdl_texture *o);
int avdl_texture_GetPixelFormat(struct avdl_texture *o);
void *avdl_texture_GetPixels(struct avdl_texture *o);

/* deprecated
void avdl_texture_bindIndexArray(struct avdl_texture *o, int index, int arraySize, struct avdl_texture *array[]);
void avdl_texture_unbindIndexArray(struct avdl_texture *o, int index);
*/

#ifdef __cplusplus
}
#endif

#endif
