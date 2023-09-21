#include "dd_image.h"
#include <stdlib.h>
#include <stdio.h>
#include "dd_log.h"
#include "avdl_assetManager.h"
#include <errno.h>

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#else
#include <png.h>
#endif

struct Subpixel {
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	GLubyte *pixels;
	#else
	float *pixels;
	#endif
	int offset_x;
	int offset_y;
	int width;
	int height;
};

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

	#if !defined( AVDL_DIRECT3D11 )
	dd_da_init(&o->subpixels, sizeof(struct Subpixel));
	#endif
}

#if defined( AVDL_DIRECT3D11 )
extern avdl_texture_id avdl_graphics_loadDDS(char *filename);
extern FILE* avdl_filetomesh_openFile(char* filename);
#endif

void dd_image_load_png(struct dd_image *img, const char *filename) {

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else

	img->tex = avdl_graphics_loadDDS(filename);
	/*
	img->width = 10;
	img->height = 10;
	img->pixelFormat = 0;
	img->pixels = malloc(sizeof(float) * img->width * img->height * 3);
	for (int x = 0; x < img->width; x++)
		for (int y = 0; y < img->height; y++) {
			int index = (y * img->width * 3) + x * 3;
			//img->pixels[(y * img->width * 3) + x * 3 + 0] = x * 0.1;
			img->pixels[index + 0] = x * 0.1;
			img->pixels[index + 1] = 0;
			//img->pixels[(y * img->width * 3) + x * 3 + 2] = y * 0.1;
			img->pixels[index + 2] = y * 0.1;
		}
		*/
	return;

	// check signature
	#if defined( AVDL_DIRECT3D11 )
	FILE* fp = avdl_filetomesh_openFile(filename);
	//FILE* fp = avdl_filetomesh_openFile("assets/button.ply");
	#else
	FILE* fp = fopen(filename, "rb");
	#endif;
	if (!fp) {
		//dd_log("dd_image_load_png: error opening file: '%s': '%s'", filename, strerror(errno));
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
	//png_read_info(png_ptr, info_ptr);

	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bit_depth = 0;
	int color_type = 0;
	int interlace_type = 0;
	int compression_type = 0;
	int filter_method = 0;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	//dd_log("%dx%d %d %d | %d %d %d", width, height, bit_depth, color_type, interlace_type, compression_type, filter_method);

	#if defined( AVDL_DIRECT3D11)
	img->pixelFormat = 0;
	#else
	img->pixelFormat = GL_RGB;
	#endif
	img->width = width;
	img->height = height;
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	// grayscale images
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		#if defined( AVDL_DIRECT3D11)
		img->pixelFormat = 0;
		#else
		img->pixelFormat = GL_RGB;
		#endif
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
		#if defined( AVDL_DIRECT3D11)
		img->pixelFormat = 0;
		#else
		img->pixelFormat = GL_RGB;
		#endif
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
		#if defined( AVDL_DIRECT3D11)
		img->pixelFormat = 0;
		#else
		img->pixelFormat = GL_RGBA;
		#endif
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

}

void dd_image_load_bmp(struct dd_image *img, const char *filename) {

	#ifdef AVDL_DIRECT3D11
	#else

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
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

	#if !defined( AVDL_DIRECT3D11 )
	for (int i = 0; i < o->subpixels.elements; i++) {
		struct Subpixel *subpixel = dd_da_get(&o->subpixels, i);
		free(subpixel->pixels);
	}
	dd_da_free(&o->subpixels);
	#endif
}

void dd_image_bind(struct dd_image *o) {

	#ifdef AVDL_DIRECT3D11
	// send texture to GPU if needed
	if (o->pixels) {
		o->tex = avdl_graphics_ImageToGpu(o->pixels, o->pixelFormat, o->width, o->height);
		free(o->pixels);
		o->pixels = 0;
	}

	// bind texture
	//if (o->openglContextId == avdl_graphics_getContextId()) {
		avdl_graphics_BindTexture(o->tex);
	//}
	#else
	// manually made texture
	if (o->openglContextId != avdl_graphics_getContextId() && !o->assetName) {

		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		o->pixelsb = malloc(sizeof(GLubyte) *4 *o->width *o->height);

		// clean the texture
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			o->pixelsb[(y*o->width*4) +x*4+0] = 255;
			o->pixelsb[(y*o->width*4) +x*4+1] = 255;
			o->pixelsb[(y*o->width*4) +x*4+2] = 255;
			o->pixelsb[(y*o->width*4) +x*4+3] = 0;
		}

		#else
		o->pixels = malloc(sizeof(float) *4 *o->width *o->height);

		// clean the texture
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			o->pixels[(y*o->width*4) +x*4+0] = 1;
			o->pixels[(y*o->width*4) +x*4+1] = 1;
			o->pixels[(y*o->width*4) +x*4+2] = 1;
			o->pixels[(y*o->width*4) +x*4+3] = 0;
		}

		#endif
		o->openglContextId = avdl_graphics_getContextId();
	}

	if (o->pixels || o->pixelsb) {

		// check tex ?
		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		o->tex = avdl_graphics_ImageToGpu(o->pixels, o->pixelFormat, o->width, o->height);
		free(o->pixels);
		o->pixels = 0;
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		o->tex = avdl_graphics_ImageToGpu(o->pixelsb, GL_RGBA, o->width, o->height);
		free(o->pixelsb);
		o->pixelsb = 0;
		#endif
	}

	#if !defined( AVDL_DIRECT3D11 )
	// update texture
	if (o->subpixels.elements > 0 && o->tex) {

		for (int i = 0; i < o->subpixels.elements; i++) {
			struct Subpixel *subpixel = dd_da_get(&o->subpixels, i);
			avdl_graphics_ImageToGpuUpdate(
				o->tex,
				subpixel->pixels,
				GL_RGBA,
				subpixel->offset_x,
				subpixel->offset_y,
				subpixel->width,
				subpixel->height
			);
			free(subpixel->pixels);
		}
		dd_da_empty(&o->subpixels);
	}
	#endif

	// texture is valid in this opengl context, bind it
	if (o->openglContextId == avdl_graphics_getContextId()) {
		avdl_graphics_BindTexture(o->tex);
	}
	// texture was in a previous opengl context, reload it
	else {
		o->tex = 0;
		if (o->assetName) {
			o->set(o, o->assetName, o->assetType);
		}
		else {
			//dd_log("error state?");
		}
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
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_add(o, AVDL_ASSETMANAGER_TEXTURE, filename, type);
	#else
	o->openglContextId = avdl_graphics_getContextId();
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_add(o, AVDL_ASSETMANAGER_TEXTURE, filename, type);
	#endif
}

void dd_image_addSubpixels(struct dd_image *o, void *pixels, int pixel_format, int x, int y, int w, int h) {

	#if defined( AVDL_DIRECT3D11 )
	return;
	#else

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	GLubyte *pixelsf = pixels;
	#else
	float *pixelsf = pixels;
	#endif

	struct Subpixel subpixel;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	subpixel.pixels = malloc(sizeof(GLubyte) *4 *w *h);
	#else
	subpixel.pixels = malloc(sizeof(float) *4 *w *h);
	#endif
	for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++) {
		int index = j *w *4 +i *4;
		subpixel.pixels[index +0] = pixelsf[index +0];
		subpixel.pixels[index +1] = pixelsf[index +1];
		subpixel.pixels[index +2] = pixelsf[index +2];
		subpixel.pixels[index +3] = pixelsf[index +3];
	}
	subpixel.offset_x = x;
	subpixel.offset_y = y;
	subpixel.width = w;
	subpixel.height = h;

	dd_da_add(&o->subpixels, &subpixel);

	#endif

}
