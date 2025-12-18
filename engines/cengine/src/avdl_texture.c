#include "avdl_texture.h"
#include <stdlib.h>
#include <stdio.h>
#include "shared/avdl_log.h"
#include "avdl_assetManager.h"
#include <errno.h>
#include "avdl_graphics.h"
#include "shared/avdl_dynamic_array.h"
#include "shared/avdl_math.h"
#include <string.h>

#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
#include <png.h>
#endif

#if defined( AVDL_DIRECT3D11 )
#include <windows.h>
#elif defined(AVDL_WINDOWS)
#include <windows.h>
extern HANDLE updateDrawMutex;
#elif defined( AVDL_LINUX ) || defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <pthread.h>
#include <unistd.h>
extern pthread_mutex_t updateDrawMutex;
extern pthread_mutex_t jniMutex;
#endif

struct Subpixel {
	avdl_graphics_ubyte *pixels;
	int offset_x;
	int offset_y;
	int width;
	int height;
};

struct avdl_texture_data {

	// pixels and their format
	enum avdl_graphics_format_internal formatInternal;
	enum avdl_graphics_format format;
	avdl_graphics_ubyte *pixels;

	// dimensions
	int width;
	int height;

	// caching data, filename is used to identify different textures
	struct avdl_string filename;
	int cacheIndex;
	int uses;

	// id for when it's stored by the graphics library
	avdl_texture_id tex;
	int graphicsContextId;
};

static struct avdl_texture_data *GetDataFromFile(const char *filename);
static int SetData(struct avdl_texture *o, struct avdl_texture_data *data);
static int CleanData(struct avdl_texture *o);

void avdl_texture_create(struct avdl_texture *o) {
	o->data = 0;
	//o->dirtyTexture = 0;
	avdl_dynamic_array_init(&o->subpixels, sizeof(struct Subpixel));

	o->clean = avdl_texture_clean;

}

#if defined( AVDL_DIRECT3D11 )
extern avdl_texture_id avdl_graphics_loadDDS(char *filename);
extern FILE* avdl_filetomesh_openFile(char* filename);
#endif

void avdl_texture_clean(struct avdl_texture *o) {

	CleanData(o);
	avdl_dynamic_array_free(&o->subpixels);

}

void avdl_texture_bind(struct avdl_texture *o) {
	avdl_texture_bindIndex(o, 0);
}

void avdl_texture_bindIndex(struct avdl_texture *o, int index) {

	if (o->data) {
		if (o->data->pixels) {
			// check tex ?
			o->data->tex = avdl_graphics_ImageToGpu(o->data->pixels, o->data->formatInternal, o->data->format, o->data->width, o->data->height);
			free(o->data->pixels);
			o->data->pixels = 0;
		}
		// update texture
		if (avdl_dynamic_array_count(&o->subpixels) > 0 && o->data->tex) {
	
			for (int i = 0; i < avdl_dynamic_array_count(&o->subpixels); i++) {
				struct Subpixel *subpixel = avdl_dynamic_array_get(&o->subpixels, i);
				avdl_graphics_ImageToGpuUpdate(
					o->data->tex,
					subpixel->pixels,
					o->data->format,
					subpixel->offset_x,
					subpixel->offset_y,
					subpixel->width,
					subpixel->height
				);
				free(subpixel->pixels);
			}
			avdl_dynamic_array_empty(&o->subpixels);
		}
		if (o->data->graphicsContextId == avdl_graphics_getContextId() && o->data->tex) {
			avdl_graphics_BindTextureIndex(o->data->tex, index);
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
	#endif
}

void avdl_texture_unbind(struct avdl_texture *o) {
	avdl_texture_unbindIndex(o, 0);
}

void avdl_texture_unbindIndex(struct avdl_texture *o, int index) {
	avdl_graphics_BindTextureIndex(0, index);
}

void avdl_texture_Load(struct avdl_texture *o, const char *filename) {
	CleanData(o);
	avdl_assetManager_AddLoadOperation(o, filename, GetDataFromFile, SetData);
}

void avdl_texture_LoadExternal(struct avdl_texture *o, const char *filename) {
	CleanData(o);
	#ifdef AVDL_DIRECT3D11
	avdl_assetManager_AddLoadOperation(o, filename, GetDataFromFile, SetData);
	#else
	avdl_assetManager_AddLoadOperationLocal(o, filename, GetDataFromFile, SetData);
	#endif
}

void avdl_texture_addSubpixels(struct avdl_texture *o, void *pixels, int offset_x, int offset_y, int w, int h) {

	#if defined( AVDL_DIRECT3D11 )
	return;
	#else

	if (!o->data) {
		avdl_log("avdl_texture_addSubpixels: no texture, can't add subpixels");
		return;
	}

	avdl_graphics_ubyte *pixelsUByte = pixels;

	// texture not uploaded yet, update in-place
	if (o->data->pixels) {
		for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++) {
			int ry = y;
			int index = (ry*o->data->width*4) +x*4+0 +offset_x*4 +(offset_y*o->data->width*4);
			int indexPixel = y *w *4 +x*4;
			o->data->pixels[index+0] = pixelsUByte[indexPixel +0];
			o->data->pixels[index+1] = pixelsUByte[indexPixel +1];
			o->data->pixels[index+2] = pixelsUByte[indexPixel +2];
			o->data->pixels[index+3] = pixelsUByte[indexPixel +3];
		}
		return;
	}

	// update subtexture
	struct Subpixel subpixel;
	subpixel.pixels = malloc(sizeof(avdl_graphics_ubyte) *4 *w *h);
	for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++) {
		int index = j *w *4 +i *4;
		subpixel.pixels[index +0] = pixelsUByte[index +0];
		subpixel.pixels[index +1] = pixelsUByte[index +1];
		subpixel.pixels[index +2] = pixelsUByte[index +2];
		subpixel.pixels[index +3] = pixelsUByte[index +3];
	}
	subpixel.offset_x = offset_x;
	subpixel.offset_y = offset_y;
	subpixel.width = w;
	subpixel.height = h;

	avdl_dynamic_array_push(&o->subpixels, &subpixel);

	#endif

}

int avdl_texture_isLoaded(struct avdl_texture *o) {
	return o->data && o->data->pixels;
}

int avdl_texture_CreateTexture(struct avdl_texture *o, int width, int height, enum avdl_graphics_format_internal formatInternal, enum avdl_graphics_format format) {

	CleanData(o);
	//o->dirtyTexture = 1;

	o->data = malloc(sizeof(struct avdl_texture_data));
	o->data->width = width;
	o->data->height = height;
	o->data->formatInternal = formatInternal;
	o->data->format = format;
	o->data->pixels = malloc(sizeof(avdl_graphics_ubyte) *4 *o->data->width *o->data->height);
	o->data->graphicsContextId = avdl_graphics_getContextId();
	o->data->tex = 0;
	o->data->uses = 0;
	o->data->cacheIndex = -1;

	avdl_string_create(&o->data->filename);
	avdl_string_SetMaxCharacters(&o->data->filename, 1024);
	avdl_string_cat(&o->data->filename, "manual_font_texture");

	// clean the texture
	for (int x = 0; x < o->data->width ; x++)
	for (int y = 0; y < o->data->height; y++) {
		o->data->pixels[(y*o->data->width*4) +x*4+0] = 255;
		o->data->pixels[(y*o->data->width*4) +x*4+1] = 255;
		o->data->pixels[(y*o->data->width*4) +x*4+2] = 255;
		o->data->pixels[(y*o->data->width*4) +x*4+3] = 0;
	}
	return 0;
}

int avdl_texture_GetWidth(struct avdl_texture *o) {
	if (o->data) {
		return o->data->width;
	}
	return 0;
}

int avdl_texture_GetHeight(struct avdl_texture *o) {
	if (o->data) {
		return o->data->height;
	}
	return 0;
}

int avdl_texture_GetPixelFormat(struct avdl_texture *o) {
	if (o->data) {
		return o->data->format;
	}
	return 0;
}

void *avdl_texture_GetPixels(struct avdl_texture *o) {
	if (o->data) {
		return o->data->pixels;
	}
	return 0;
}

struct avdl_dynamic_array textureCache;

struct avdl_texture_data *FindTexture(const char *filename) {
	for (int i = 0; i < avdl_dynamic_array_count(&textureCache); i++) {
		struct avdl_texture_data *t = avdl_dynamic_array_getDeref(&textureCache, i);
		if (strcmp(avdl_string_toCharPtr(&t->filename), filename) == 0) {
			return t;
		}
	}
	return 0;
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <jni.h>
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass *clazz;
extern jmethodID BitmapMethodId;
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
extern AAssetManager *aassetManager;
#endif

static struct avdl_texture_data *GetDataFromFile(const char *filename) {

	struct avdl_texture_data *o;

	// if found cached texture, return that
	o = FindTexture(filename);
	if (o) {
		return o;
	}

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )

	#if !defined( AVDL_QUEST2 )
	/*
	 * attempt to get hold of a valid jni
	 * will most likely matter during
	 * screen orientation
	 */
	pthread_mutex_lock(&jniMutex);
	while (!jvm) {
		pthread_mutex_unlock(&jniMutex);
		//avdl_log("sleeping");
		sleep(1);
		pthread_mutex_lock(&jniMutex);
	}
	#endif

	JNIEnv *env;

	int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
			avdl_log("avdl: failed to attach thread for new world");
		}
	// thread already attached to jni
	} else if (getEnvStat == JNI_OK) {
	// wrong version
	} else if (getEnvStat == JNI_EVERSION) {
		avdl_log("avdl: GetEnv: version not supported");
	}

	jstring *parameter = (*env)->NewStringUTF(env, filename);
	jobjectArray result = (jstring)(*(*env)->CallStaticObjectMethod)(env, clazz, BitmapMethodId, parameter);

	if (result) {

		// the first object describes the size of the texture
		const jintArray size  = (*(*env)->GetObjectArrayElement)(env, result, 0);
		const jint *sizeValues = (*(*env)->GetIntArrayElements)(env, size, 0);

		// the second object describes the pixels
		const jintArray pixels  = (*(*env)->GetObjectArrayElement)(env, result, 1);
		const jint *pixelValues = (*(*env)->GetIntArrayElements)(env, pixels, 0);

		int width = sizeValues[0];
		int height = sizeValues[1];
		avdl_graphics_ubyte *pixelsb = malloc(sizeof(avdl_graphics_ubyte) *width *height *4);

		/*
		 * read pixels into a new array
		 * for some reason the texture returned is flipped on the y axis
		 * so it can be parsed in reverse, until it's more clear why this
		 * happens
		 */
		jsize len = (*env)->GetArrayLength(env, pixels);
		for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int index = ((y *width) +x);
			int indexReverse = (((height -(y+1)) *width) +x);
			pixelsb[indexReverse*4 +0] = dd_math_pow(((pixelValues[index] & 0x00FF0000) >> 16) /255.0, 2.2) *255;
			pixelsb[indexReverse*4 +1] = dd_math_pow(((pixelValues[index] & 0x0000FF00) >>  8) /255.0, 2.2) *255;
			pixelsb[indexReverse*4 +2] = dd_math_pow(((pixelValues[index] & 0x000000FF)      ) /255.0, 2.2) *255;
			pixelsb[indexReverse*4 +3] = dd_math_pow(((pixelValues[index] & 0xFF000000) >> 24) /255.0, 2.2) *255;
		}
		}

		(*env)->ReleaseIntArrayElements(env, size, sizeValues, JNI_ABORT);
		(*env)->ReleaseIntArrayElements(env, pixels, pixelValues, JNI_ABORT);

		o = malloc(sizeof(struct avdl_texture_data));
		avdl_string_create(&o->filename);
		avdl_string_SetMaxCharacters(&o->filename, 1024);
		avdl_string_cat(&o->filename, filename);
		if (!avdl_string_isValid(&o->filename)) {
			avdl_log("avdl: AssetManager: Unable to construct filename for texture: %s", filename);
			return 0;
		}
		o->format = AVDL_GRAPHICS_RGBA;
		o->formatInternal = AVDL_GRAPHICS_RGBA8;
		o->pixels = pixelsb;
		o->width = width;
		o->height = height;
	}
	else {
		avdl_log("avdl: error loading texture: %s", filename);
		if (jvm && getEnvStat == JNI_EDETACHED) {
			(*jvm)->DetachCurrentThread(jvm);
		}
		return 0;
	}

	if (jvm && getEnvStat == JNI_EDETACHED) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	#else

	o = malloc(sizeof(struct avdl_texture_data));
	if (!o) {
		avdl_log("error allocating memory for texture data: %s", filename);
		return 0;
	}

	//t = malloc(sizeof(struct avdl_texture_data));
	avdl_string_create(&o->filename);
	avdl_string_SetMaxCharacters(&o->filename, 1024);
	avdl_string_cat(&o->filename, filename);
	if (!avdl_string_isValid(&o->filename)) {
		avdl_log("avdl: AssetManager: Unable to construct filename for texture: %s", filename);
		return 0;
	}

	// check signature
	#if defined( AVDL_DIRECT3D11 )
	FILE* fp = avdl_filetomesh_openFile(filename);
	#else
	FILE* fp = fopen(filename, "rb");
	#endif
	if (!fp) {
		avdl_log("avdl: AssetManager: LoadTexturePNG: error opening file: '%s': '%s'", filename, strerror(errno));
		return 0;
	}
	unsigned char header[9];
	fread(header, 1, 8, fp);
	header[8] = '\0';
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png)
	{
		avdl_log("avdl: AssetManager: LoadTexturePNG: error reading asset file signature: '%s'", filename);
		fclose(fp);
		return 0;
	}

	// create struct pointer
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		avdl_log("avdl: AssetManager: LoadTexturePNG: error parsing file: '%s'", filename);
		fclose(fp);
		return 0;
	}

	// create info pointer
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		avdl_log("avdl: AssetManager: LoadTexturePNG: error creating info struct from file: '%s'", filename);
		return 0;
	}

	// init
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	// transformations
	int color_type = png_get_color_type(png_ptr, info_ptr);

	// convert palette to RGB
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}
	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bit_depth = 0;
	int interlace_type = 0;
	int compression_type = 0;
	int filter_method = 0;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	//avdl_log("%dx%d %d %d | %d %d %d", width, height, bit_depth, color_type, interlace_type, compression_type, filter_method);

	o->width = width;
	o->height = height;
	png_bytep *row_pointers = malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
		row_pointers[y] = malloc(png_get_rowbytes(png_ptr, info_ptr));
	}
	png_read_image(png_ptr, row_pointers);

	// grayscale images
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		o->format = AVDL_GRAPHICS_RED;
		o->formatInternal = AVDL_GRAPHICS_R8;
		avdl_graphics_ubyte *pixels = malloc(sizeof(avdl_graphics_ubyte) *o->width *o->height *1);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*1) +x*1+0] = dd_math_pow(row_pointers[ry][x] /255.0, 2.2) *255;
		}
		o->pixels = pixels;
	}
	else
	// RGB images
	if (color_type == PNG_COLOR_TYPE_RGB) {
		o->format = AVDL_GRAPHICS_RGB;
		o->formatInternal = AVDL_GRAPHICS_RGB8;
		avdl_graphics_ubyte *pixels = malloc(sizeof(avdl_graphics_ubyte) *o->width *o->height *3);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*3) +x*3+0] = dd_math_pow(row_pointers[ry][x*3+0] /255.0, 2.2) *255;
			pixels[(y*width*3) +x*3+1] = dd_math_pow(row_pointers[ry][x*3+1] /255.0, 2.2) *255;
			pixels[(y*width*3) +x*3+2] = dd_math_pow(row_pointers[ry][x*3+2] /255.0, 2.2) *255;
		}
		o->pixels = pixels;
	}
	else
	// RGBA images
	if (color_type == PNG_COLOR_TYPE_RGBA) {
		o->format = AVDL_GRAPHICS_RGBA;
		o->formatInternal = AVDL_GRAPHICS_RGBA8;
		avdl_graphics_ubyte *pixels = malloc(sizeof(avdl_graphics_ubyte) *o->width *o->height *4);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*4) +x*4+0] = dd_math_pow(row_pointers[ry][x*4+0] /255.0, 2.2) *255;
			pixels[(y*width*4) +x*4+1] = dd_math_pow(row_pointers[ry][x*4+1] /255.0, 2.2) *255;
			pixels[(y*width*4) +x*4+2] = dd_math_pow(row_pointers[ry][x*4+2] /255.0, 2.2) *255;
			pixels[(y*width*4) +x*4+3] = dd_math_pow(row_pointers[ry][x*4+3] /255.0, 2.2) *255;
		}
		o->pixels = pixels;
	}
	// unsupported format
	else {
		// free pointers
		for(int y = 0; y < height; y++) {
			free(row_pointers[y]);
		}
		free(row_pointers);

		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		avdl_log("avdl: AssetManager: LoadTexturePNG: error while parsing '%s': unsupported format: color_type: %d", filename, color_type);
		return 0;
	}

	// free pointers
	for(int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}
	free(row_pointers);

	// clean-up
	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	#endif

	// add newly created texture to texture cache
	o->cacheIndex = avdl_dynamic_array_count(&textureCache);
	o->graphicsContextId = avdl_graphics_getContextId();
	o->uses = 0;
	o->tex = 0;
	avdl_dynamic_array_push(&textureCache, &o);

	return o;
}

static int SetData(struct avdl_texture *o, struct avdl_texture_data *data) {
	CleanData(o);
	data->uses++;
	o->data = data;
	return 0;
}

// used when data are not used anymore by a texture,
// if data is not used by any other texture, delete it
static void ReduceDataCacheUses(struct avdl_texture_data *t) {
	t->uses--;
	if (t->uses == 0) {
		for (int i = 0; i < avdl_dynamic_array_count(&textureCache); i++) {
			struct avdl_texture_data *tempTex = avdl_dynamic_array_getDeref(&textureCache, i);
			if (tempTex == t) {
				avdl_string_clean(&t->filename);
				if (t->pixels) {
					free(t->pixels);
					t->pixels = 0;
				}
				free(t);
				avdl_dynamic_array_remove(&textureCache, 1, i);
				break;
			}
		}
	}
}

// cleans up data (if any) and resets data pointer
static int CleanData(struct avdl_texture *o) {

	if (!o->data) {
		return 0;
	}

	// cached - do not clean
	if (o->data->cacheIndex >= 0) {
		ReduceDataCacheUses(o->data);
		o->data = 0;
		return 0;
	}

	if (o->data->pixels) {
		free(o->data->pixels);
		o->data->pixels = 0;
	}

	if (o->data->tex) {
		avdl_graphics_DeleteTexture(o->data->tex);
	}
	avdl_string_clean(&o->data->filename);
	free(o->data);
	o->data = 0;

	#if !defined( AVDL_DIRECT3D11 )
	for (int i = 0; i < avdl_dynamic_array_count(&o->subpixels); i++) {
		struct Subpixel *subpixel = avdl_dynamic_array_get(&o->subpixels, i);
		free(subpixel->pixels);
	}
	avdl_dynamic_array_empty(&o->subpixels);
	#endif

	return 0;
}
