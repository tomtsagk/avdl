#include "avdl_assetManager.h"
#include "dd_dynamic_array.h"
#include "dd_filetomesh.h"
#include "avdl_log.h"
#include <string.h>
#include <stdlib.h>
#include "dd_meshTexture.h"
#include "avdl_mesh.h"
#include <stdio.h>
#include "dd_game.h"

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

struct dd_dynamic_array meshesToLoad;
struct dd_dynamic_array meshesLoading;

int assetManagerLoading;

static float desiredLoadedPercentage;
int totalAssets;
int totalAssetsLoaded;
static int lockLoading;

static int interruptLoading;

void avdl_assetManager_init() {
	dd_da_init(&meshesToLoad , sizeof(struct dd_meshToLoad));
	dd_da_init(&meshesLoading, sizeof(struct dd_meshToLoad));
	assetManagerLoading = 0;
	lockLoading = 0;
	interruptLoading = 0;
	desiredLoadedPercentage = 1.0;
}

void avdl_assetManager_deinit() {
	dd_da_free(&meshesToLoad );
	dd_da_free(&meshesLoading);
}

void avdl_assetManager_add(void *object, int meshType, const char *assetname, int type) {
	if (lockLoading) {
		return;
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
	dd_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
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
	//printf("add asset: %s\n", meshToLoad.filename);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#endif
	dd_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

}

void avdl_assetManager_addLocal(void *object, int meshType, const char *assetname, int type) {
	if (lockLoading) {
		return;
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
	dd_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.type = type;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	strcpy(meshToLoad.filename, assetname);
	//printf("add asset: %s\n", meshToLoad.filename);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#endif
	dd_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

}

void avdl_assetManager_loadAssets() {

	// load assets here
	//avdl_log("meshes to load: %d", meshesLoading.elements);
	for (int i = 0; i < meshesLoading.elements; i++) {
		struct dd_meshToLoad *m = dd_da_get(&meshesLoading, i);
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

			struct dd_image *mesh = m->object;

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
			struct dd_image *mesh = m->object;
			if (m->type == AVDL_IMAGETYPE_BMP) {
				dd_image_load_bmp(mesh, m->filename);
			}
			else
			if (m->type == AVDL_IMAGETYPE_PNG) {
				if (dd_image_load_png(mesh, m->filename) != 0) {
					avdl_log("avdl: error loading texture %s", m->filename);
				}
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
		if (interruptLoading) break;
		//avdl_log("assets loaded: %d / %d", totalAssetsLoaded, totalAssets);
		#ifdef AVDL_DIRECT3D11
		#elif defined( AVDL_WINDOWS )
		ReleaseMutex(updateDrawMutex);
		#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
		pthread_mutex_unlock(&updateDrawMutex);
		#endif

		//avdl_log("done");
	}
	dd_da_empty(&meshesLoading);
	//avdl_log("finished all loading");

	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_WINDOWS )
	WaitForSingleObject(updateDrawMutex, INFINITE);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
	pthread_mutex_lock(&updateDrawMutex);
	#endif
	assetManagerLoading = 0;
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

	dd_da_copy(&meshesLoading, &meshesToLoad);
	dd_da_empty(&meshesToLoad);

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
	dd_da_empty(&meshesToLoad);
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
