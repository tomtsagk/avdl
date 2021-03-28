#include "avdl_assetManager.h"
#include "dd_dynamic_array.h"
#include "dd_filetomesh.h"
#include <pthread.h>
#include "dd_log.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dd_meshTexture.h"

#if DD_PLATFORM_ANDROID
#include <jni.h>
extern JavaVM* jvm;
extern jclass *clazz;
#endif

struct dd_dynamic_array meshesToLoad;
struct dd_dynamic_array meshesLoading;

extern pthread_mutex_t updateDrawMutex;
extern pthread_mutex_t jniMutex;

int assetManagerLoading;

static float desiredLoadedPercentage;
static int totalAssets;
static int totalAssetsLoaded;
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

void avdl_assetManager_add(void *object, int meshType, const char *assetname) {
	if (lockLoading) {
		return;
	}
	#if DD_PLATFORM_ANDROID
	if (assetManagerLoading) {
		dd_log("error add new asset while loading: %s", assetname);
		return;
	}

	struct dd_meshToLoad meshToLoad;
	meshToLoad.mesh = object;
	meshToLoad.meshType = meshType;
	strcpy(&meshToLoad.filename, assetname);
	dd_da_add(&meshesToLoad, &meshToLoad);
	#endif

}

void avdl_assetManager_loadAssets() {

	#if DD_PLATFORM_ANDROID
	// load assets here
	dd_log("meshes to load: %d", meshesLoading.elements);
	for (int i = 0; i < meshesLoading.elements; i++) {
		struct dd_meshToLoad *m = dd_da_get(&meshesLoading, i);
		dd_log("loading asset: %s", m->filename);
		//dd_log("loading asset type: %d", m->meshType);

		/*
		 * attempt to get hold of a valid jni
		 * will most likely matter during
		 * screen orientation
		 */
		pthread_mutex_lock(&jniMutex);
		while (!jvm) {
			pthread_mutex_unlock(&jniMutex);
			//dd_log("sleeping");
			sleep(1);
			pthread_mutex_lock(&jniMutex);
		}

		#if DD_PLATFORM_ANDROID
		JNIEnv *env;
		int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);

		if (getEnvStat == JNI_EDETACHED) {
			if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
				dd_log("avdl: failed to attach thread for new world");
			}
		} else if (getEnvStat == JNI_OK) {
			dd_log("avdl: thread attached to JNI");
		} else if (getEnvStat == JNI_EVERSION) {
			dd_log("avdl: GetEnv: version not supported");
		}
		#endif

		// load texture
		if (m->meshType == AVDL_ASSETMANAGER_TEXTURE) {
			struct dd_meshTexture *mesh = m->mesh;

			jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "ReadBitmap", "(Ljava/lang/String;)[Ljava/lang/Object;");
			jstring *parameter = (*env)->NewStringUTF(env, m->filename);
			jobjectArray result = (jstring)(*(*env)->CallStaticObjectMethod)(env, clazz, MethodID, parameter);

			if (result) {

				// the first object describes the size of the texture
				const jintArray size  = (*(*env)->GetObjectArrayElement)(env, result, 0);
				const jint *sizeValues = (*(*env)->GetIntArrayElements)(env, size, 0);

				// the second object describes the pixels
				const jintArray pixels  = (*(*env)->GetObjectArrayElement)(env, result, 1);
				const jint *pixelValues = (*(*env)->GetIntArrayElements)(env, pixels, 0);

				int width = sizeValues[0];
				int height = sizeValues[1];
				GLubyte *pixelsb = malloc(sizeof(GLubyte) *width *height *3);

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
					pixelsb[indexReverse*3 +0] = (pixelValues[index] & 0x00FF0000) >> 16;
					pixelsb[indexReverse*3 +1] = (pixelValues[index] & 0x0000FF00) >>  8;
					pixelsb[indexReverse*3 +2] = (pixelValues[index] & 0x000000FF);
				}
				}

				(*env)->ReleaseIntArrayElements(env, size, sizeValues, JNI_ABORT);
				(*env)->ReleaseIntArrayElements(env, pixels, pixelValues, JNI_ABORT);

				pthread_mutex_lock(&updateDrawMutex);
				mesh->img.width = width;
				mesh->img.height = height;
				mesh->img.pixelsb = pixelsb;
				pthread_mutex_unlock(&updateDrawMutex);
			}
			//dd_log("done: %s", m->filename);
		}
		// load mesh
		else {

			// get string from asset (in java)
			jmethodID MethodID = (*(*env)->GetStaticMethodID)(env, clazz, "ReadPly", "(Ljava/lang/String;I)[Ljava/lang/Object;");
			jstring *parameter = (*env)->NewStringUTF(env, m->filename);
			jint parameterSettings = DD_FILETOMESH_SETTINGS_POSITION;
			if (m->meshType == AVDL_ASSETMANAGER_MESHCOLOUR) {
				parameterSettings |= DD_FILETOMESH_SETTINGS_COLOUR;
				if (m->meshType == AVDL_ASSETMANAGER_MESHTEXTURE) {
					parameterSettings |= DD_FILETOMESH_SETTINGS_TEX_COORD;
				}
			}
			jobjectArray result = (jstring)(*(*env)->CallStaticObjectMethod)(env, clazz, MethodID, parameter, &parameterSettings);

			/*
			 * Reading the asset was successfull,
			 * load the asset with it.
			 */
			if (result && (m->meshType == AVDL_ASSETMANAGER_MESHCOLOUR)) {

				struct dd_meshColour *mesh = m->mesh;

				// the first object describes the size of the texture
				const jintArray pos  = (*(*env)->GetObjectArrayElement)(env, result, 0);
				const jfloat *posValues = (*(*env)->GetFloatArrayElements)(env, pos, 0);

				const jintArray col  = (*(*env)->GetObjectArrayElement)(env, result, 1);
				const jint *colValues = (*(*env)->GetIntArrayElements)(env, col, 0);

				jsize len = (*env)->GetArrayLength(env, pos) /3;
				pthread_mutex_lock(&updateDrawMutex);
				mesh->parent.vcount = len;
				mesh->parent.v = malloc(sizeof(float) *len *3);
				mesh->c = malloc(sizeof(float) *len *4);
				for (int i = 0; i < len; i++) {
					mesh->parent.v[i*3+0] = posValues[i*3+0];
					mesh->parent.v[i*3+1] = posValues[i*3+1];
					mesh->parent.v[i*3+2] = posValues[i*3+2];
					mesh->c[i*4+0] = (float) (colValues[i*3+0] /255.0);
					mesh->c[i*4+1] = (float) (colValues[i*3+1] /255.0);
					mesh->c[i*4+2] = (float) (colValues[i*3+2] /255.0);
					mesh->c[i*4+3] = 1;
				}
				mesh->parent.dirtyVertices = 1;
				mesh->dirtyColours = 1;
				pthread_mutex_unlock(&updateDrawMutex);

				(*env)->ReleaseFloatArrayElements(env, pos, posValues, JNI_ABORT);
				(*env)->ReleaseIntArrayElements(env, col, colValues, JNI_ABORT);

			}
			else
			if (result) {

				struct dd_mesh *mesh = m->mesh;

				// the first object describes the size of the texture
				const jintArray pos  = (*(*env)->GetObjectArrayElement)(env, result, 0);
				const jfloat *posValues = (*(*env)->GetFloatArrayElements)(env, pos, 0);

				jsize len = (*env)->GetArrayLength(env, pos) /3;
				pthread_mutex_lock(&updateDrawMutex);
				mesh->vcount = len;
				mesh->v = malloc(sizeof(float) *len *3);
				for (int i = 0; i < len; i++) {
					mesh->v[i*3+0] = posValues[i*3+0];
					mesh->v[i*3+1] = posValues[i*3+1];
					mesh->v[i*3+2] = posValues[i*3+2];
				}
				mesh->dirtyVertices = 1;
				pthread_mutex_unlock(&updateDrawMutex);

				(*env)->ReleaseFloatArrayElements(env, pos, posValues, JNI_ABORT);

			}
		}

		#if DD_PLATFORM_ANDROID
		if (jvm && getEnvStat == JNI_EDETACHED) {
			//dd_log("detach thread");
			(*jvm)->DetachCurrentThread(jvm);
		}
		#endif

		pthread_mutex_unlock(&jniMutex);

		pthread_mutex_lock(&updateDrawMutex);
		totalAssetsLoaded++;
		if (interruptLoading) break;
		//dd_log("assets loaded: %d / %d", totalAssetsLoaded, totalAssets);
		pthread_mutex_unlock(&updateDrawMutex);

		//dd_log("done");
	}
	dd_da_empty(&meshesLoading);
	//dd_log("finished all loading");
	#endif

	pthread_mutex_lock(&updateDrawMutex);
	assetManagerLoading = 0;
	pthread_mutex_unlock(&updateDrawMutex);

}

void avdl_assetManager_clean() {
}

/*
 * load assets async
 */
static pthread_t loadAssetsThread = NULL;

void *load_assets_thread_function(void *data) {
	avdl_assetManager_loadAssets();
	pthread_exit(NULL);
}

void avdl_assetManager_loadAssetsAsync() {
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
	pthread_create(&loadAssetsThread, NULL, load_assets_thread_function, 0);
	pthread_detach(loadAssetsThread);
}

int avdl_assetManager_isLoading() {
	return assetManagerLoading;
}

int avdl_assetManager_isReady() {
	if (totalAssetsLoaded == totalAssets) {
		return 1;
	}

	return (float) totalAssetsLoaded / (float) totalAssets >= desiredLoadedPercentage;
}

void avdl_assetManager_setPercentage(float percentage) {
	desiredLoadedPercentage = percentage;
}

int avdl_assetManager_hasAssets() {
	return meshesToLoad.elements > 0;
}

void avdl_assetManager_lockLoading() {
	lockLoading = 1;
}

void avdl_assetManager_unlockLoading() {
	lockLoading = 0;
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
