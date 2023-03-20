#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#include "dd_opengl.h"

enum AVDL_IMAGETYPE {
	AVDL_IMAGETYPE_PNG,
	AVDL_IMAGETYPE_BMP,
};

struct dd_image {
	GLuint tex;
	int width, height;
	float *pixels;
	GLubyte *pixelsb;
	const char *assetName;
	int assetType;
	int openglContextId;

	int pixelFormat;

	void (*bind)(struct dd_image *o);
	void (*unbind)(struct dd_image *o);
	void (*clean)(struct dd_image *o);
	void (*set)(struct dd_image *o, const char *filename, int type);
};

void dd_image_create(struct dd_image *o);

#if defined(WIN32) || defined(_WIN32)
void dd_image_load_bmp(struct dd_image *img, const wchar_t *filename);
#else
void dd_image_load_bmp(struct dd_image *img, const char *filename);
#endif
void dd_image_load_png(struct dd_image *img, const char *filename);

void dd_image_bind(struct dd_image *o);
void dd_image_unbind(struct dd_image *o);
void dd_image_set(struct dd_image *o, const char *filename, int type);

void dd_image_clean(struct dd_image *o);

#endif
