#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#include "dd_opengl.h"

struct dd_image {
	GLuint tex;
	int width, height;
	float *pixels;
	GLubyte *pixelsb;
	char *assetName;
	int openglContextId;

	void (*bind)(struct dd_image *o);
	void (*unbind)(struct dd_image *o);
	void (*clean)(struct dd_image *o);
	void (*set)(struct dd_image *o, const char *filename);
};

void dd_image_create(struct dd_image *o);

#if defined(WIN32) || defined(_WIN32)
void dd_image_load_bmp(struct dd_image *img, const wchar_t *filename);
#else
void dd_image_load_bmp(struct dd_image *img, const char *filename);
#endif
void dd_image_to_opengl(struct dd_image *img);

void dd_image_bind(struct dd_image *o);
void dd_image_unbind(struct dd_image *o);
void dd_image_set(struct dd_image *o, const char *filename);

void dd_image_clean(struct dd_image *o);

#endif
