#include "dd_image.h"
#include <stdlib.h>
#include <stdio.h>
#include "dd_log.h"
#include "avdl_assetManager.h"
#include <errno.h>

#ifdef AVDL_DIRECT3D11
#elif DD_PLATFORM_ANDROID
#else
#include <png.h>
#endif

void dd_image_create(struct dd_image *o) {
	o->tex = 0;
	o->width = 0;
	o->height = 0;
	o->pixels = 0;
	o->pixelsb = 0;
	o->assetName = 0;
	o->assetType = 0;
	o->openglContextId = -1;
	o->pixelFormat = 0;

	o->bind = dd_image_bind;
	o->unbind = dd_image_unbind;
	o->clean = dd_image_clean;
	o->set = dd_image_set;
}

void dd_image_load_png(struct dd_image *img, const char *filename) {

	#ifdef AVDL_DIRECT3D11
	#else

	#if DD_PLATFORM_ANDROID
	#else

	// check signature
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dd_log("dd_image_load_png: error opening file: '%s': '%s'", filename, strerror(errno));
		return;
	}
	char header[9];
	fread(header, 1, 8, fp);
	header[8] = '\0';
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png)
	{
		dd_log("avdl: error reading asset file signature '%s'", filename);
		fclose(fp);
		return;
	}

	//png_set_sig_bytes_read();

	// create struct pointer
	//png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr, user_error_fn, user_warning_fn);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		dd_log("avdl: error while parsing '%s'", filename);
		fclose(fp);
		return;
	}

	// create info pointer
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		dd_log("avdl: error while parsing '%s'", filename);
		return;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_type;
	int compression_type;
	int filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	//dd_log("%dx%d %d %d | %d %d %d", width, height, bit_depth, color_type, interlace_type, compression_type, filter_method);

	img->pixelFormat = GL_RGB;
	img->width = width;
	img->height = height;
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	// grayscale images
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		img->pixelFormat = GL_RGB;
		img->pixels = malloc(sizeof(float) *img->width *img->height *3);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			img->pixels[(y*width*3) +x*3+0] = row_pointers[ry][x]/ 255.0;
			img->pixels[(y*width*3) +x*3+1] = row_pointers[ry][x]/ 255.0;
			img->pixels[(y*width*3) +x*3+2] = row_pointers[ry][x]/ 255.0;
		}
	}
	else
	// RGB images
	if (color_type == PNG_COLOR_TYPE_RGB) {
		img->pixelFormat = GL_RGB;
		img->pixels = malloc(sizeof(float) *img->width *img->height *3);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			img->pixels[(y*width*3) +x*3+0] = row_pointers[ry][x*3+0]/ 255.0;
			img->pixels[(y*width*3) +x*3+1] = row_pointers[ry][x*3+1]/ 255.0;
			img->pixels[(y*width*3) +x*3+2] = row_pointers[ry][x*3+2]/ 255.0;
		}
	}
	else
	// RGBA images
	if (color_type == PNG_COLOR_TYPE_RGBA) {
		img->pixelFormat = GL_RGBA;
		img->pixels = malloc(sizeof(float) *img->width *img->height *4);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			img->pixels[(y*width*4) +x*4+0] = row_pointers[ry][x*4+0]/ 255.0;
			img->pixels[(y*width*4) +x*4+1] = row_pointers[ry][x*4+1]/ 255.0;
			img->pixels[(y*width*4) +x*4+2] = row_pointers[ry][x*4+2]/ 255.0;
			img->pixels[(y*width*4) +x*4+3] = row_pointers[ry][x*4+3]/ 255.0;
		}
	}
	// unsupported format
	else {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		dd_log("avdl: error while parsing '%s': unsupported format", filename);
		return;
	}

	// clean-up
	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	#endif

	#endif // direct3d 11

}

void dd_image_load_bmp(struct dd_image *img, const char *filename) {

	#ifdef AVDL_DIRECT3D11
	#else

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
		dd_log("dd_image_load_bmp: error opening file: '%s': '%s'", filename, strerror(errno));
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

	#endif

}

void dd_image_clean(struct dd_image *o) {
	#ifdef AVDL_DIRECT3D11
	#else
	if (o->pixels) {
		free(o->pixels);
		o->pixels = 0;
	}

	if (o->pixelsb) {
		free(o->pixelsb);
		o->pixelsb = 0;
	}

	if (o->tex) {
		avdl_graphics_DeleteTexture(o->tex);
	}
	#endif
}

void dd_image_bind(struct dd_image *o) {

	#ifdef AVDL_DIRECT3D11
	#else
	if (o->pixels || o->pixelsb) {
		avdl_graphics_ImageToGpu(o);
	}

	// texture is valid in this opengl context, bind it
	if (o->openglContextId == avdl_graphics_getContextId()) {
		avdl_graphics_BindTexture(o->tex);
	}
	// texture was in a previous opengl context, reload it
	else
	if (o->assetName) {
		o->tex = 0;
		o->set(o, o->assetName, o->assetType);
	}
	#endif
}

void dd_image_unbind(struct dd_image *o) {
	#ifdef AVDL_DIRECT3D11
	#else
	if (o->tex) {
		avdl_graphics_BindTexture(0);
	}
	#endif
}

void dd_image_set(struct dd_image *o, const char *filename, int type) {
	#ifdef AVDL_DIRECT3D11
	#else
	o->openglContextId = avdl_graphics_getContextId();
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_add(o, AVDL_ASSETMANAGER_TEXTURE, filename, type);
	#endif
}
