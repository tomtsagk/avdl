#ifndef DD_IMAGE_H
#define DD_IMAGE_H

#include "dd_opengl.h"

struct dd_image {
	GLuint tex;
	int width, height;
	float *pixels;
	GLubyte *pixelsb;
};

void dd_image_load_bmp(struct dd_image *img, const char *filename);
void dd_image_to_opengl(struct dd_image *img);

void dd_image_free(struct dd_image *img);
void dd_image_draw(struct dd_image *img);

#endif
