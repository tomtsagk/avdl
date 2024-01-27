#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_graphics.h"
#include "dd_dynamic_array.h"

enum AVDL_IMAGETYPE {
	AVDL_IMAGETYPE_PNG,
	AVDL_IMAGETYPE_BMP,
};

struct dd_image {
	#ifdef AVDL_DIRECT3D11
	avdl_texture_id tex;
	int *pixelsb;
	#else
	GLuint tex;
	GLubyte *pixelsb;
	//struct dd_dynamic_array subpixels;
	#endif
	int width, height;
	float *pixels;
	const char *assetName;
	int assetType;
	int openglContextId;

	int pixelFormat;

	void (*bind)(struct dd_image *o);
	void (*bindIndex)(struct dd_image *o, int index);
	void (*unbind)(struct dd_image *o);
	void (*clean)(struct dd_image *o);
	void (*set)(struct dd_image *o, const char *filename, int type);

	void (*addSubpixels)(struct dd_image *o, void *pixels, int pixel_format, int x, int y, int w, int h);
};

void dd_image_create(struct dd_image *o);

#ifdef AVDL_DIRECT3D11
void dd_image_load_bmp(struct dd_image *img, const char *filename);
#elif defined(WIN32) || defined(_WIN32)
void dd_image_load_bmp(struct dd_image *img, const wchar_t *filename);
#else
void dd_image_load_bmp(struct dd_image *img, const char *filename);
#endif
void dd_image_load_png(struct dd_image *img, const char *filename);

void dd_image_bind(struct dd_image *o);
void dd_image_bindIndex(struct dd_image *o, int index);
void dd_image_unbind(struct dd_image *o);
void dd_image_set(struct dd_image *o, const char *filename, int type);

void dd_image_clean(struct dd_image *o);

void dd_image_addSubpixels(struct dd_image *o, void *pixels, int pixel_format, int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif

#endif
