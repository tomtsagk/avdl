#include "dd_image.h"
#include <stdlib.h>
#include <stdio.h>
#include "dd_log.h"

void dd_image_load_bmp(struct dd_image *img, const char *filename) {

	#if DD_PLATFORM_ANDROID
	#else
	struct bmp_header {
		unsigned short int type;
		unsigned int size;
		unsigned short int reserved1, reserved2;
		unsigned int offset;
	} header;

	struct bmp_headerinfo {
		unsigned int size;
		int width, height;
		unsigned short int planes;
		unsigned short int bits;
		unsigned int compression;
		unsigned int imageSize;
		int xresolution, yresolution;
		unsigned int ncolours;
		unsigned int importantcolours;
	} headerinfo;

	// on Unix system, "r" is enough, on windows "rb" is needed
	FILE *f = fopen(filename, "rb");
	if (!f) {
		dd_log("dd_image_load_bmp: error opening file: '%s'", filename);
		exit(-1);
	}

	fread(&header.type, sizeof(unsigned short int), 1, f);
	fread(&header.size, sizeof(unsigned int), 1, f);
	fread(&header.reserved1, sizeof(unsigned short int), 1, f);
	fread(&header.reserved2, sizeof(unsigned short int), 1, f);
	fread(&header.offset, sizeof(unsigned int), 1, f);

	if (fread(&headerinfo, sizeof(struct bmp_headerinfo), 1, f) != 1) {
		dd_log("dd_image_load_bmp: error reading info header: '%s'", filename);
	}

	fseek(f, header.offset, SEEK_SET);

	img->width = headerinfo.width;
	img->height = headerinfo.height;
	img->pixels = malloc(sizeof(float) *img->width *img->height *3);
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	for (int h = 0; h < headerinfo.height; h++)
	for (int w = 0; w < headerinfo.width; w++) {
		fread(&b, sizeof(unsigned char), 1, f);
		fread(&g, sizeof(unsigned char), 1, f);
		fread(&r, sizeof(unsigned char), 1, f);

		int index = h*headerinfo.width*3 +w*3;
		img->pixels[index+0] = r/255.0;
		img->pixels[index+1] = g/255.0;
		img->pixels[index+2] = b/255.0;
	}

	fclose(f);
	#endif

}

void dd_image_to_opengl(struct dd_image *img) {

	glGenTextures(1, &img->tex);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#if DD_PLATFORM_NATIVE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_FLOAT, img->pixels);
	#elif DD_PLATFORM_ANDROID
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->pixelsb);
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	#if DD_PLATFORM_NATIVE
	free(img->pixels);
	img->pixels = 0;
	#elif DD_PLATFORM_ANDROID
	free(img->pixelsb);
	img->pixelsb = 0;
	#endif

}

void dd_image_free(struct dd_image *img) {
	glDeleteTextures(1, &img->tex);
}

void dd_image_draw(struct dd_image *img) {
}
