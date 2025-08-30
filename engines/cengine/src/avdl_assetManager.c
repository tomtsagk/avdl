#include "avdl_assetManager.h"
#include "shared/avdl_dynamic_array.h"
#include "dd_filetomesh.h"
#include "shared/avdl_log.h"
#include <string.h>
#include <stdlib.h>
#include "dd_meshTexture.h"
#include "avdl_mesh.h"
#include <stdio.h>
#include "dd_game.h"
#include <errno.h>
#include "dd_math.h"

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_DIRECT3D11 )
#else
#include <png.h>
#endif

void avdl_assetManager_loadAssets();

#ifdef AVDL_DIRECT3D11
#include <windows.h>
#elif defined(AVDL_WINDOWS)
#include <windows.h>
extern HANDLE updateDrawMutex;
#else
#include <pthread.h>
#include <unistd.h>
extern pthread_mutex_t updateDrawMutex;
extern pthread_mutex_t jniMutex;
#endif

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

/*
 * load assets async
 */
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
static pthread_t loadAssetsThread = 0;

void *load_assets_thread_function(void *data) {
	avdl_assetManager_loadAssets();
	pthread_exit(NULL);
}
#elif defined( AVDL_WINDOWS )
#include <windows.h>

HANDLE thread;

DWORD WINAPI ThreadFunc(void* data) {
	avdl_assetManager_loadAssets();
	return 0;
}
#endif

struct avdl_dynamic_array meshesToLoad;
struct avdl_dynamic_array meshesLoading;
struct avdl_dynamic_array textureCache;

int assetManagerLoading;

static float desiredLoadedPercentage;
int totalAssets;
int totalAssetsLoaded;
static int lockLoading;

static int interruptLoading;
static int exitLoading;

static int LoadTexturePNG(struct avdl_assetManager_texture *o, const char *filename);
static struct avdl_assetManager_texture *FindTexture(const char *filename);

void avdl_assetManager_init() {
	avdl_da_init(&meshesToLoad , sizeof(struct dd_meshToLoad));
	avdl_da_init(&meshesLoading, sizeof(struct dd_meshToLoad));
	assetManagerLoading = 0;
	lockLoading = 0;
	interruptLoading = 0;
	desiredLoadedPercentage = 1.0;
	exitLoading = 0;

	// texture cache
	avdl_da_init(&textureCache, sizeof(struct avdl_assetManager_texture *));
}

void avdl_assetManager_deinit() {
	exitLoading = 1;
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_WINDOWS )
	WaitForSingleObject(updateDrawMutex, INFINITE);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	avdl_da_free(&meshesToLoad );
	avdl_da_free(&meshesLoading);

	if (avdl_da_count(&textureCache) > 0) {
		avdl_log("%d texture(s) were not cleaned", avdl_da_count(&textureCache));
		/*
		for (int i = 0; i < avdl_da_count(&textureCache); i++) {
			struct avdl_assetManager_texture *t = avdl_da_getDeref(&textureCache, i);
			avdl_log("texture: %s", avdl_string_toCharPtr(&t));
		}
		*/
	}
	avdl_da_free(&textureCache);
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_WINDOWS )
	ReleaseMutex(updateDrawMutex);
	CloseHandle(thread);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_mutex_unlock(&updateDrawMutex);
	#endif
}

int avdl_assetManager_add(void *object, int meshType, const char *assetname, int type, int (*callback)(void *obj, void *data)) {
	if (lockLoading) {
		return -1;
	}
	//#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	/*
	if (assetManagerLoading) {
		avdl_log("error add new asset while loading: %s", assetname);
		return;
	}
	*/
	#ifdef AVDL_DIRECT3D11
	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
	meshToLoad.callback = callback;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	strcpy(meshToLoad.filename, avdl_getProjectLocation());
	strcat(meshToLoad.filename, GAME_ASSET_PREFIX);
	strcat(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s", meshToLoad.filename);
	#endif
	avdl_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

	return 0;
}

void avdl_assetManager_remove(int index) {
	avdl_log("remove asset: %d", index);
}

int avdl_assetManager_addLocal(void *object, int meshType, const char *assetname, int type, int (*callback)(void *obj, void *data)) {
	if (lockLoading) {
		return -1;
	}
	//#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	/*
	if (assetManagerLoading) {
		avdl_log("error add new asset while loading: %s", assetname);
		return;
	}
	*/
	#ifdef AVDL_DIRECT3D11
	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
	meshToLoad.callback = callback;
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
	meshToLoad.callback = callback;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	strcpy(meshToLoad.filename, assetname);
	//printf("add asset: %s\n", meshToLoad.filename);
	//avdl_log("add asset: %s", meshToLoad.filename);
	#endif
	avdl_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

	return 0;

}

void avdl_assetManager_loadAssets() {

	// load assets here
	//avdl_log("meshes to load: %d", meshesLoading.elements);
	for (int i = 0; i < meshesLoading.elements; i++) {
		struct dd_meshToLoad *m = avdl_da_get(&meshesLoading, i);
		//avdl_log("loading asset: %s", m->filename);
		//wprintf(L"loading asset: %lS", m->filenameW);
		//avdl_log("loading asset type: %d", m->meshType);

		// load texture
		if (m->meshType == AVDL_ASSETMANAGER_TEXTURE) {
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

			struct avdl_texture *mesh = m->object;

			jstring *parameter = (*env)->NewStringUTF(env, m->filename);
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
				GLubyte *pixelsb = malloc(sizeof(GLubyte) *width *height *4);

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
					pixelsb[indexReverse*4 +0] = (pixelValues[index] & 0x00FF0000) >> 16;
					pixelsb[indexReverse*4 +1] = (pixelValues[index] & 0x0000FF00) >>  8;
					pixelsb[indexReverse*4 +2] = (pixelValues[index] & 0x000000FF);
					pixelsb[indexReverse*4 +3] = (pixelValues[index] & 0xFF000000) >> 24;
				}
				}

				(*env)->ReleaseIntArrayElements(env, size, sizeValues, JNI_ABORT);
				(*env)->ReleaseIntArrayElements(env, pixels, pixelValues, JNI_ABORT);

				#if defined( AVDL_DIRECT3D11 )
				#elif defined( AVDL_WINDOWS )
				WaitForSingleObject(updateDrawMutex, INFINITE);
				#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
				pthread_mutex_lock(&updateDrawMutex);
				#endif
				mesh->width = width;
				mesh->height = height;
				mesh->pixelsb = pixelsb;
				#if defined( AVDL_DIRECT3D11 )
				#elif defined( AVDL_WINDOWS )
				ReleaseMutex(updateDrawMutex);
				#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
				pthread_mutex_unlock(&updateDrawMutex);
				#endif
			}
			else {
				avdl_log("avdl: error loading texture: %s", m->filename);
			}
			//avdl_log("done: %s", m->filename);

			if (jvm && getEnvStat == JNI_EDETACHED) {
				(*jvm)->DetachCurrentThread(jvm);
			}
			//#endif
			#else
			struct avdl_texture *mesh = m->object;
			if (m->type == AVDL_IMAGETYPE_BMP) {
				avdl_texture_load_bmp(mesh, m->filename);
			}
			else
			if (m->type == AVDL_IMAGETYPE_PNG) {

				// attempt to find texture from cache
				struct avdl_assetManager_texture *t = 0;
				t = FindTexture(m->filename);

				// texture not found - load a new one
				if (t == 0) {
					t = malloc(sizeof(struct avdl_assetManager_texture));
					avdl_string_create(&t->filename, 1024);
					avdl_string_cat(&t->filename, m->filename);
					if (!avdl_string_isValid(&t->filename)) {
						avdl_log("avdl: AssetManager: Unable to construct filename for texture: %s", m->filename);
						continue;
					}
					if (LoadTexturePNG(t, m->filename) != 0) {
						avdl_log("avdl: AssetManager: Unable to load texture: %s", m->filename);
						continue;
					}
					t->index = avdl_da_count(&textureCache);
					t->graphicsContextId = avdl_graphics_getContextId();
					t->uses = 0;
					t->tex = 0;
					avdl_da_push(&textureCache, &t);
				}

				#if defined( AVDL_DIRECT3D11 )
				#elif defined( AVDL_WINDOWS )
				WaitForSingleObject(updateDrawMutex, INFINITE);
				#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
				pthread_mutex_lock(&updateDrawMutex);
				#endif
				if (exitLoading) {
					#if defined( AVDL_DIRECT3D11 )
					#elif defined( AVDL_WINDOWS )
					ReleaseMutex(updateDrawMutex);
					#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
					pthread_mutex_unlock(&updateDrawMutex);
					#endif
					return;
				}
				if (m->callback) {
					t->uses++;
					if (m->callback(mesh, t) != 0) {
						avdl_log("avdl: AssetManager: error loading texture %s", m->filename);
					}
				}
				#if defined( AVDL_DIRECT3D11 )
				#elif defined( AVDL_WINDOWS )
				ReleaseMutex(updateDrawMutex);
				#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
				pthread_mutex_unlock(&updateDrawMutex);
				#endif
			}
			#endif
		}
		// load mesh
		else {
			// mesh
			if (m->meshType == AVDL_ASSETMANAGER_MESH2) {
				struct avdl_mesh *mesh = m->object;
				struct dd_loaded_mesh lm = {0};
				if (dd_filetomesh(&lm, m->filename,
					DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR, m->type) == -1) {
					// error loading file
					avdl_log("avdl: error loading mesh2: %s", m->filename);
				}
				else {
					#if defined( AVDL_DIRECT3D11 )
					#elif defined( AVDL_WINDOWS )
					WaitForSingleObject(updateDrawMutex, INFINITE);
					#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
					pthread_mutex_lock(&updateDrawMutex);
					#endif
					if (exitLoading) {
						#if defined( AVDL_DIRECT3D11 )
						#elif defined( AVDL_WINDOWS )
						ReleaseMutex(updateDrawMutex);
						#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
						pthread_mutex_unlock(&updateDrawMutex);
						#endif
						return;
					}
					mesh->LoadFromLoadedMesh(mesh, &lm);
					#if defined( AVDL_DIRECT3D11 )
					#elif defined( AVDL_WINDOWS )
					ReleaseMutex(updateDrawMutex);
					#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
					pthread_mutex_unlock(&updateDrawMutex);
					#endif
				}
			}
			else
			if (m->meshType == AVDL_ASSETMANAGER_MESH) {
				struct dd_mesh *mesh = m->object;
				dd_mesh_clean(mesh);
				struct dd_loaded_mesh lm;
				if (dd_filetomesh(&lm, m->filename, DD_FILETOMESH_SETTINGS_POSITION, DD_PLY) == -1) {
					// error
				}
				else {
					mesh->vcount = lm.vcount;
					mesh->v = lm.v;
					mesh->dirtyVertices = 1;
				}
			}
			else
			// mesh colour
			if (m->meshType == AVDL_ASSETMANAGER_MESHCOLOUR) {
				struct dd_meshColour *mesh = m->object;
				dd_meshColour_clean(mesh);
				struct dd_loaded_mesh lm;
				if (dd_filetomesh(&lm, m->filename,
					DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR, DD_PLY) == -1) {
					// error loading file
				}
				else {
					mesh->parent.vcount = lm.vcount;
					mesh->parent.v = lm.v;
					mesh->parent.dirtyVertices = 1;
					mesh->c = lm.c;
					mesh->dirtyColours = 1;
				}
			}
			else
			// mesh texture
			if (m->meshType == AVDL_ASSETMANAGER_MESHTEXTURE) {
				struct dd_meshTexture *mesh = m->object;
				dd_meshTexture_clean(mesh);
				struct dd_loaded_mesh lm;
				if (dd_filetomesh(&lm, m->filename,
					DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR
					| DD_FILETOMESH_SETTINGS_TEX_COORD, DD_PLY) == -1) {
					// error
				}
				else {
					mesh->parent.parent.vcount = lm.vcount;
					mesh->parent.parent.v = lm.v;
					mesh->parent.parent.dirtyVertices = 1;
					mesh->parent.c = lm.c;
					mesh->parent.dirtyColours = 1;
					mesh->t = lm.t;
					mesh->dirtyTextures = 1;
				}
			}
		}

		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		pthread_mutex_unlock(&jniMutex);
		#endif

		#ifdef AVDL_DIRECT3D11
		#elif defined( AVDL_WINDOWS )
		WaitForSingleObject(updateDrawMutex, INFINITE);
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
		pthread_mutex_lock(&updateDrawMutex);
		#endif

		totalAssetsLoaded++;
		if (interruptLoading) {
			#if defined( AVDL_DIRECT3D11 )
			#elif defined( AVDL_WINDOWS )
			ReleaseMutex(updateDrawMutex);
			#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
			pthread_mutex_unlock(&updateDrawMutex);
			#endif
			break;
		}
		//avdl_log("assets loaded: %d / %d", totalAssetsLoaded, totalAssets);
		#ifdef AVDL_DIRECT3D11
		#elif defined( AVDL_WINDOWS )
		ReleaseMutex(updateDrawMutex);
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
		pthread_mutex_unlock(&updateDrawMutex);
		#endif

		//avdl_log("done");
	}
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_WINDOWS )
	WaitForSingleObject(updateDrawMutex, INFINITE);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	if (exitLoading) {
		#ifdef AVDL_DIRECT3D11
		#elif defined( AVDL_WINDOWS )
		ReleaseMutex(updateDrawMutex);
		CloseHandle(thread);
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
		pthread_mutex_unlock(&updateDrawMutex);
		#endif
		return;
	}
	avdl_da_empty(&meshesLoading);
	//avdl_log("finished all loading");
	assetManagerLoading = 0;
	loadAssetsThread = 0;
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_WINDOWS )
	ReleaseMutex(updateDrawMutex);
	CloseHandle(thread);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_mutex_unlock(&updateDrawMutex);
	#endif
}

void avdl_assetManager_loadAll() {
	if (assetManagerLoading) return;

	avdl_da_copy(&meshesLoading, &meshesToLoad);
	avdl_da_empty(&meshesToLoad);

	totalAssets = meshesLoading.elements;
	totalAssetsLoaded = 0;
	assetManagerLoading = 1;

	/*
	if (loadAssetsThread) {
		pthread_join(&loadAssetsThread, NULL);
		loadAssetsThread = 0;
	}
	*/
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_create(&loadAssetsThread, NULL, load_assets_thread_function, 0);
	pthread_detach(loadAssetsThread); // do not wait for thread result code
	#elif defined( AVDL_WINDOWS )
	HANDLE thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
	#else
	avdl_assetManager_loadAssets();
	#endif
}

int avdl_assetManager_isLoading() {
	return assetManagerLoading;
}

int avdl_assetManager_isReady() {
	return totalAssetsLoaded == totalAssets;

	/*
	if (totalAssetsLoaded == totalAssets) {
		return 1;
	}

	return (float) totalAssetsLoaded / (float) totalAssets >= desiredLoadedPercentage;
	*/
}

int avdl_assetManager_hasAssetsToLoad() {
	return meshesToLoad.elements > 0;
}

void avdl_assetManager_lockLoading() {
	lockLoading = 1;
}

void avdl_assetManager_unlockLoading() {
	lockLoading = 0;
}

float avdl_assetManager_getLoadedProportion() {
	if (totalAssets <= 0) {
		return 1.0;
	}

	return (float) totalAssetsLoaded / (float) totalAssets;
}

void avdl_assetManager_clear() {
	/*
	avdl_da_empty(&meshesToLoad);
	if (loadAssetsThread) {
		interruptLoading = 1;
		pthread_join(&loadAssetsThread, NULL);
		interruptLoading = 0;
		loadAssetsThread = 0;
	}
	*/
}

void avdl_assetManager_setPercentage(float percentage) {
	desiredLoadedPercentage = percentage;
}

static struct avdl_assetManager_texture *FindTexture(const char *filename) {
	for (int i = 0; i < avdl_da_count(&textureCache); i++) {
		struct avdl_assetManager_texture *t = avdl_da_getDeref(&textureCache, i);
		if (strcmp(avdl_string_toCharPtr(&t->filename), filename) == 0) {
			return t;
		}
	}
	return 0;
}

static int LoadTexturePNG(struct avdl_assetManager_texture *o, const char *filename) {

	// check signature
	#if defined( AVDL_DIRECT3D11 )
	FILE* fp = avdl_filetomesh_openFile(filename);
	#else
	FILE* fp = fopen(filename, "rb");
	#endif
	if (!fp) {
		avdl_log("avdl: AssetManager: LoadTexturePNG: error opening file: '%s': '%s'", filename, strerror(errno));
		return -1;
	}
	unsigned char header[9];
	fread(header, 1, 8, fp);
	header[8] = '\0';
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png)
	{
		avdl_log("avdl: AssetManager: LoadTexturePNG: error reading asset file signature: '%s'", filename);
		fclose(fp);
		return -1;
	}

	//png_set_sig_bytes_read();

	// create struct pointer
	//png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr, user_error_fn, user_warning_fn);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		avdl_log("avdl: AssetManager: LoadTexturePNG: error parsing file: '%s'", filename);
		fclose(fp);
		return -1;
	}

	// create info pointer
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		avdl_log("avdl: AssetManager: LoadTexturePNG: error creating info struct from file: '%s'", filename);
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
	o->pixelFormat = 0;
	#else
	o->pixelFormat = GL_RGB;
	#endif
	o->width = width;
	o->height = height;
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	// grayscale images
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		#if defined( AVDL_DIRECT3D11)
		o->pixelFormat = 0;
		#else
		o->pixelFormat = GL_RGB;
		#endif
		float *pixels = malloc(sizeof(float) *o->width *o->height *3);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*3) +x*3+0] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+1] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+2] = dd_math_pow(row_pointers[ry][x]/ 255.0, 2.2);
		}
		o->pixels = pixels;
	}
	else
	// RGB images
	if (color_type == PNG_COLOR_TYPE_RGB) {
		#if defined( AVDL_DIRECT3D11)
		o->pixelFormat = 0;
		#else
		o->pixelFormat = GL_RGB;
		#endif
		float *pixels = malloc(sizeof(float) *o->width *o->height *3);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*3) +x*3+0] = dd_math_pow(row_pointers[ry][x*3+0]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+1] = dd_math_pow(row_pointers[ry][x*3+1]/ 255.0, 2.2);
			pixels[(y*width*3) +x*3+2] = dd_math_pow(row_pointers[ry][x*3+2]/ 255.0, 2.2);
		}
		o->pixels = pixels;
	}
	else
	// RGBA images
	if (color_type == PNG_COLOR_TYPE_RGBA) {
		#if defined( AVDL_DIRECT3D11)
		o->pixelFormat = 0;
		#else
		o->pixelFormat = GL_RGBA;
		#endif
		float *pixels = malloc(sizeof(float) *o->width *o->height *4);
		for (int x = 0; x < o->width ; x++)
		for (int y = 0; y < o->height; y++) {
			int ry = o->height-1 -y;
			pixels[(y*width*4) +x*4+0] = dd_math_pow(row_pointers[ry][x*4+0]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+1] = dd_math_pow(row_pointers[ry][x*4+1]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+2] = dd_math_pow(row_pointers[ry][x*4+2]/ 255.0, 2.2);
			pixels[(y*width*4) +x*4+3] = dd_math_pow(row_pointers[ry][x*4+3]/ 255.0, 2.2);
		}
		o->pixels = pixels;
	}
	// unsupported format
	else {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		avdl_log("avdl: AssetManager: LoadTexturePNG: error while parsing '%s': unsupported format: color_type: %d", filename, color_type);
		return -1;
	}

	// clean-up
	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return 0;
}

void avdl_assetManager_CleanTexture(struct avdl_assetManager_texture *t) {
	t->uses--;
	if (t->uses == 0) {
		for (int i = 0; i < avdl_da_count(&textureCache); i++) {
			struct avdl_assetManager_texture *tempTex = avdl_da_getDeref(&textureCache, i);
			if (tempTex == t) {
				avdl_string_clean(&t->filename);
				if (t->pixels) {
					free(t->pixels);
					t->pixels = 0;
				}
				free(t);
				avdl_da_remove(&textureCache, 1, i);
				break;
			}
		}
	}
}
