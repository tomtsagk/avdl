#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#include "dd_opengl.h"

struct dd_image {
	GLuint tex;
	int width, height;
	float *pixels;
	GLubyte *pixelsb;
};

#if defined(WIN32) || defined(_WIN32)
void dd_image_load_bmp(struct dd_image *img, const wchar_t *filename);
#else
void dd_image_load_bmp(struct dd_image *img, const char *filename);
#endif
void dd_image_to_opengl(struct dd_image *img);

void dd_image_free(struct dd_image *img);
void dd_image_draw(struct dd_image *img);

#endif
