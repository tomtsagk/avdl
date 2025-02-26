#include "avdl_texture.h"
#include <stdlib.h>
#include <stdio.h>
#include "avdl_log.h"
#include "avdl_assetManager.h"
#include <errno.h>
#include "dd_math.h"

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
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

void avdl_texture_create(struct avdl_texture *o) {
	o->tex = 0;
	o->width = 0;
	o->height = 0;
	o->pixels = 0;
	o->pixelsb = 0;
	o->assetName = 0;
	o->assetType = 0;
	o->openglContextId = -1;
	o->pixelFormat = 0;
	o->texture = 0;
	o->dirtyTexture = 0;

	o->bind = avdl_texture_bind;
	o->bindIndex = avdl_texture_bindIndex;
	o->bindIndexArray = avdl_texture_bindIndexArray;
	o->unbind = avdl_texture_unbind;
	o->unbindIndex = avdl_texture_unbindIndex;
	o->unbindIndexArray = avdl_texture_unbindIndexArray;
	o->clean = avdl_texture_clean;
	o->set = avdl_texture_set;
	o->isLoaded = avdl_texture_isLoaded;

	#if !defined( AVDL_DIRECT3D11 )
	dd_da_init(&o->subpixels, sizeof(struct Subpixel));
	#endif
}

#if defined( AVDL_DIRECT3D11 )
extern avdl_texture_id avdl_graphics_loadDDS(char *filename);
extern FILE* avdl_filetomesh_openFile(char* filename);
#endif

int avdl_texture_load_png(struct avdl_texture *img, const char *filename) {

	avdl_texture_clean(img);

	avdl_log("load png manual: %s", filename);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#elif defined( AVDL_DIRECT3D11 )
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
	#else

	// check signature
	#if defined( AVDL_DIRECT3D11 )
	FILE* fp = avdl_filetomesh_openFile(filename);
	//FILE* fp = avdl_filetomesh_openFile("assets/button.ply");
	#else
	FILE* fp = fopen(filename, "rb");
	#endif
	if (!fp) {
		avdl_log("avdl_texture_load_png: error opening file: '%s': '%s'", filename, strerror(errno));
		return -1;
	}
	char header[9];
	fread(header, 1, 8, fp);
	header[8] = '\0';
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png)
	{
		avdl_log("avdl: error reading asset file signature '%s'", filename);
		fclose(fp);
		return -1;
	}

	//png_set_sig_bytes_read();

	// create struct pointer
	//png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr, user_error_fn, user_warning_fn);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		avdl_log("avdl: error while parsing '%s'", filename);
		fclose(fp);
		return -1;
	}

	// create info pointer
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		avdl_log("avdl: error while parsing '%s'", filename);
		return -1;
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

	//avdl_log("%dx%d %d %d | %d %d %d", width, height, bit_depth, color_type, interlace_type, compression_type, filter_method);

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
		float *pixels = malloc(sizeof(float) *img->width *img->height *3);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			pixels[(y*width*3) +x*3+0] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+1] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+2] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
		}
		img->pixels = pixels;
	}
	else
	// RGB images
	if (color_type == PNG_COLOR_TYPE_RGB) {
		#if defined( AVDL_DIRECT3D11)
		img->pixelFormat = 0;
		#else
		img->pixelFormat = GL_RGB;
		#endif
		float *pixels = malloc(sizeof(float) *img->width *img->height *3);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			pixels[(y*width*3) +x*3+0] = dd_math_pow(row_pointers[ry][x*3+0]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+1] = dd_math_pow(row_pointers[ry][x*3+1]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+2] = dd_math_pow(row_pointers[ry][x*3+2]/ 255.0, 2.2);
		}
		img->pixels = pixels;
	}
	else
	// RGBA images
	if (color_type == PNG_COLOR_TYPE_RGBA) {
		#if defined( AVDL_DIRECT3D11)
		img->pixelFormat = 0;
		#else
		img->pixelFormat = GL_RGBA;
		#endif
		float *pixels = malloc(sizeof(float) *img->width *img->height *4);
		for (int x = 0; x < img->width ; x++)
		for (int y = 0; y < img->height; y++) {
			int ry = img->height-1 -y;
			pixels[(y*width*4) +x*4+0] = dd_math_pow(row_pointers[ry][x*4+0]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+1] = dd_math_pow(row_pointers[ry][x*4+1]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+2] = dd_math_pow(row_pointers[ry][x*4+2]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+3] = dd_math_pow(row_pointers[ry][x*4+3]/ 255.0, 2.2);
		}
		img->pixels = pixels;
	}
	// unsupported format
	else {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		avdl_log("avdl: error while parsing '%s': unsupported format: color_type: %d", filename, color_type);
		return -1;
	}

	// clean-up
	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	#endif

	return 0;

}

int avdl_texture_load_FromAsset(struct avdl_texture *img, struct avdl_assetManager_texture *t) {
/*
	img->pixelFormat = t->pixelFormat;
	img->pixels = t->pixels;
	img->width = t->width;
	img->height = t->height;
	*/
	img->texture = t;
	img->dirtyTexture = 0;
	return 0;
}

void avdl_texture_load_bmp(struct avdl_texture *img, const char *filename) {

	avdl_log("load bmp: %s", filename);

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
		avdl_log("avdl_texture_load_bmp: error opening file: '%s': '%s'", filename, strerror(errno));
		exit(-1);
	}

	fread(&header.type, sizeof(unsigned short int), 1, f);
	fread(&header.size, sizeof(unsigned int), 1, f);
	fread(&header.reserved1, sizeof(unsigned short int), 1, f);
	fread(&header.reserved2, sizeof(unsigned short int), 1, f);
	fread(&header.offset, sizeof(unsigned int), 1, f);

	if (fread(&headerinfo, sizeof(struct bmp_headerinfo), 1, f) != 1) {
		avdl_log("avdl_texture_load_bmp: error reading info header: '%s'", filename);
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

void avdl_texture_clean(struct avdl_texture *o) {

	// cached texture - do not clean
	if (o->texture && !o->dirtyTexture) {
		avdl_assetManager_CleanTexture(o->texture);
	}

	if (o->texture && o->dirtyTexture) {
		if (o->texture->pixels) {
			free(o->texture->pixels);
			o->texture->pixels = 0;
		}
		if (o->texture->tex) {
			avdl_graphics_DeleteTexture(o->texture->tex);
		}
	}

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

void avdl_texture_bind(struct avdl_texture *o) {
	avdl_texture_bindIndex(o, 0);
}

void avdl_texture_bindIndex(struct avdl_texture *o, int index) {

	if (o->texture) {
		if (o->texture->pixels) {
			// check tex ?
			#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
			o->texture->tex = avdl_graphics_ImageToGpu(o->texture->pixels, o->texture->pixelFormat, o->texture->width, o->texture->height);
			free(o->texture->pixels);
			o->texture->pixels = 0;
			#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			#endif
		}
		if (o->texture->graphicsContextId == avdl_graphics_getContextId() && o->texture->tex) {
			avdl_graphics_BindTextureIndex(o->texture->tex, index);
		}
		return;
	}

	#ifdef AVDL_DIRECT3D11
	/*
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
	*/
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
		float *pixels = malloc(sizeof(float) *o->width *o->height *4);

		// clean the texture
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			pixels[(y*o->width*4) +x*4+0] = 1;
			pixels[(y*o->width*4) +x*4+1] = 1;
			pixels[(y*o->width*4) +x*4+2] = 1;
			pixels[(y*o->width*4) +x*4+3] = 0;
		}
		o->pixelFormat = GL_RGB;
		o->pixels = pixels;

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
		avdl_graphics_BindTextureIndex(o->tex, index);
	}
	// texture was in a previous opengl context, reload it
	else {
		o->tex = 0;
		if (o->assetName) {
			o->set(o, o->assetName, o->assetType);
		}
		else {
			//avdl_log("error state?");
		}
	}
	#endif
}

void avdl_texture_bindIndexArray(struct avdl_texture *o, int index, int arraySize, struct avdl_texture *array[]) {

	if (o->texture) {
		int isLoaded = 1;
		for (int i = 0; i < arraySize; i++) {
			if (!array[i]->texture || !array[i]->texture->pixels) {
				isLoaded = 0;
				break;
			}
		}
		if (isLoaded) {
			
			// check tex ?
			#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
			o->texture->tex = avdl_graphics_ImageArrayToGpuStart(o->texture->pixels, o->texture->pixelFormat, o->texture->width, o->texture->height, arraySize);
			for (int i = 0; i < arraySize; i++) {
				avdl_graphics_ImageArrayToGpuInstance(array[i]->texture->pixels, o->texture->pixelFormat, o->texture->width, o->texture->height, i);
			}
			avdl_graphics_ImageArrayToGpuEnd();
			free(o->texture->pixels);
			o->texture->pixels = 0;
			#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			#endif
		}

		// texture is valid in this opengl context, bind it
		if (o->texture->graphicsContextId == avdl_graphics_getContextId() && o->texture->tex) {
			avdl_graphics_BindTextureArrayIndex(o->texture->tex, index);
		}
		return;
	}

	#ifdef AVDL_DIRECT3D11
	#else
	int isLoaded = 1;
	for (int i = 0; i < arraySize; i++) {
		if (!array[i]->isLoaded(array[i])) {
			//avdl_log("one or more images from array are not loaded");
			isLoaded = 0;
			break;
		}
	}

	if (isLoaded) {
		// check tex ?
		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		o->tex = avdl_graphics_ImageArrayToGpuStart(o->pixels, o->pixelFormat, o->width, o->height, arraySize);
		for (int i = 0; i < arraySize; i++) {
			avdl_graphics_ImageArrayToGpuInstance(array[i]->pixels, o->pixelFormat, o->width, o->height, i);
		}
		avdl_graphics_ImageArrayToGpuEnd();
		//free(o->pixels);
		o->pixels = 0;
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		/*
		o->tex = avdl_graphics_ImageToGpu(o->pixelsb, GL_RGBA, o->width, o->height);
		free(o->pixelsb);
		o->pixelsb = 0;
		*/
		#endif
	}

	// texture is valid in this opengl context, bind it
	if (o->openglContextId == avdl_graphics_getContextId()) {
		avdl_graphics_BindTextureArrayIndex(o->tex, index);
	}
	/*
	// texture was in a previous opengl context, reload it
	else {
		o->tex = 0;
		if (o->assetName) {
			o->set(o, o->assetName, o->assetType);
		}
		else {
			//avdl_log("error state?");
		}
	}
	*/
	#endif
}

void avdl_texture_unbind(struct avdl_texture *o) {
	avdl_texture_unbindIndex(o, 0);
}

void avdl_texture_unbindIndex(struct avdl_texture *o, int index) {
	#ifdef AVDL_DIRECT3D11
	#else
	if (o->tex) {
		avdl_graphics_BindTextureIndex(0, index);
	}
	#endif
}

void avdl_texture_unbindIndexArray(struct avdl_texture *o, int index) {
	#ifdef AVDL_DIRECT3D11
	#else
	if (o->tex) {
		avdl_graphics_BindTextureArrayIndex(0, index);
	}
	#endif
}

void avdl_texture_set(struct avdl_texture *o, const char *filename, int type) {
	#ifndef AVDL_DIRECT3D11
	o->openglContextId = avdl_graphics_getContextId();
	#endif
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_add(o, AVDL_ASSETMANAGER_TEXTURE, filename, type, avdl_texture_load_FromAsset);
}

void avdl_texture_setLocal(struct avdl_texture *o, const char *filename, int type) {
	#ifdef AVDL_DIRECT3D11
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_add(o, AVDL_ASSETMANAGER_TEXTURE, filename, type);
	#else
	o->openglContextId = avdl_graphics_getContextId();
	o->assetName = filename;
	o->assetType = type;
	avdl_assetManager_addLocal(o, AVDL_ASSETMANAGER_TEXTURE, filename, type);
	#endif
}

void avdl_texture_addSubpixels(struct avdl_texture *o, void *pixels, int pixel_format, int x, int y, int w, int h) {

	avdl_log("add subpixels?");

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

	dd_da_push(&o->subpixels, &subpixel);

	#endif

}

int avdl_texture_isLoaded(struct avdl_texture *o) {
	return o->pixels || o->pixelsb;
}

void avdl_texture_cleanNonGpuData(struct avdl_texture *o) {
	if (o->pixels) {
		//free(o->pixels);
		o->pixels = 0;
	}
}
