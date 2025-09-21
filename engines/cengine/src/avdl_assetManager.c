#include "avdl_assetManager.h"
#include "shared/avdl_dynamic_array.h"
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

struct avdl_texture_data;

struct avdl_dynamic_array meshesToLoad;
struct avdl_dynamic_array meshesLoading;
extern struct avdl_dynamic_array textureCache;

int assetManagerLoading;

static float desiredLoadedPercentage;
int totalAssets;
int totalAssetsLoaded;
static int lockLoading;

static int interruptLoading;
static int exitLoading;

struct avdl_string avdl_custom_asset_location;

void avdl_assetManager_init() {
	avdl_da_init(&meshesToLoad , sizeof(struct dd_meshToLoad));
	avdl_da_init(&meshesLoading, sizeof(struct dd_meshToLoad));
	assetManagerLoading = 0;
	lockLoading = 0;
	interruptLoading = 0;
	desiredLoadedPercentage = 1.0;
	exitLoading = 0;

	// texture cache
	avdl_da_init(&textureCache, sizeof(struct avdl_texture_data *));

	//avdl_log("init avdl custom asset location");
	avdl_string_create(&avdl_custom_asset_location);
	avdl_string_SetMaxCharacters(&avdl_custom_asset_location, 1024);
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
		for (int i = 0; i < avdl_da_count(&textureCache); i++) {
			struct avdl_texture_data *t = avdl_da_getDeref(&textureCache, i);
			avdl_log("texture: %s", avdl_string_toCharPtr(&t));
		}
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

int avdl_assetManager_AddLoadOperation(void *object, const char *assetname, void *(*loadOperation)(const char *), int (*callback)(void *obj, void *data)) {
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
	/*
	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	*/
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = -1;
	meshToLoad.loadOperation = loadOperation;
	meshToLoad.callback = callback;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	struct avdl_string assetstr;
	avdl_string_create(&assetstr);
	avdl_string_SetMaxCharacters(&assetstr, 1024);
	avdl_string_cat(&assetstr, assetname);
	avdl_string_replaceEnding(&assetstr, ".png", "");
	strcpy(meshToLoad.filename, avdl_string_toCharPtr(&assetstr));
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	strcpy(meshToLoad.filename, avdl_getProjectLocation());
	strcat(meshToLoad.filename, "/");
	strcat(meshToLoad.filename, GAME_ASSET_PREFIX);
	strcat(meshToLoad.filename, "/assets/");
	strcat(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s", meshToLoad.filename);
	#endif
	avdl_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

	return 0;
}

int avdl_assetManager_AddLoadOperationLocal(void *object, const char *assetname, void *(*loadOperation)(const char *), int (*callback)(void *obj, void *data)) {
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
	/*
	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	*/
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = -1;
	meshToLoad.loadOperation = loadOperation;
	meshToLoad.callback = callback;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	char prefix[] = "assets/";
	if (strncmp(assetname, prefix, strlen(prefix)) == 0) {
		char *assetnameShort = assetname +strlen(prefix);
		char buffer[1024];
		strcpy(buffer, assetnameShort);
		char *period = strstr(buffer, ".");
		if (strcmp(period, ".png") == 0) {
			period[0] = '\0';
		}
		strcpy(meshToLoad.filename, buffer);
	}
	else {
		strcpy(meshToLoad.filename, assetname);
	}
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	if (!avdl_string_IsEmpty(&avdl_custom_asset_location)) {
		strcpy(meshToLoad.filename, avdl_string_toCharPtr(&avdl_custom_asset_location));
		strcat(meshToLoad.filename, "/assets/");
		strcat(meshToLoad.filename, assetname);
	}
	else {
		strcpy(meshToLoad.filename, "assets/");
		strcat(meshToLoad.filename, assetname);
	}
	//avdl_log("add load operation asset: %s", meshToLoad.filename);
	#endif
	avdl_da_push(&meshesToLoad, &meshToLoad);
	//#endif

	#endif

	return 0;
}

int avdl_assetManager_add(void *object, int meshType, const char *assetname, int (*callback)(void *obj, void *data)) {
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
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.callback = callback;
	meshToLoad.loadOperation = 0;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, "assets/");
	strcat(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	char buffer[1024];
	strcpy(buffer, assetname);
	char *period = strstr(buffer, ".");
	if (strcmp(period, ".png") == 0) {
		period[0] = '\0';
	}
	strcpy(meshToLoad.filename, buffer);
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	strcpy(meshToLoad.filename, avdl_getProjectLocation());
	strcat(meshToLoad.filename, "/");
	strcat(meshToLoad.filename, GAME_ASSET_PREFIX);
	strcat(meshToLoad.filename, "/assets/");
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

int avdl_assetManager_addLocal(void *object, int meshType, const char *assetname, int (*callback)(void *obj, void *data)) {
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
	meshToLoad.callback = callback;
	strcpy_s(meshToLoad.filename, 300, assetname);
	avdl_da_push(&meshesToLoad, &meshToLoad);
	#else

	struct dd_meshToLoad meshToLoad;
	meshToLoad.object = object;
	meshToLoad.meshType = meshType;
	meshToLoad.callback = callback;
	meshToLoad.loadOperation = 0;
	#if defined(_WIN32) || defined(WIN32)
	strcpy(meshToLoad.filename, "assets/");
	strcat(meshToLoad.filename, assetname);
	//avdl_log("add asset: %s\n", meshToLoad.filename);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(meshToLoad.filename, assetname);
	//avdl_log("add android asset: %s\n", meshToLoad.filename);
	#else
	if (!avdl_string_IsEmpty(&avdl_custom_asset_location)) {
		strcpy(meshToLoad.filename, avdl_string_toCharPtr(&avdl_custom_asset_location));
		strcat(meshToLoad.filename, "/assets/");
		strcat(meshToLoad.filename, assetname);
	}
	else {
		strcpy(meshToLoad.filename, "assets/");
		strcat(meshToLoad.filename, assetname);
	}
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

		// abstract loading
		if (m->loadOperation) {

			void *data = m->loadOperation(m->filename);
			if (!data) {
				avdl_log("avdl: AssetManager: failed to execute load operation: %s", m->filename);
				continue;
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
				if (m->callback(m->object, data) != 0) {
					avdl_log("avdl: AssetManager: failed to execute callback: %s", m->filename);
					#if defined( AVDL_DIRECT3D11 )
					#elif defined( AVDL_WINDOWS )
					ReleaseMutex(updateDrawMutex);
					#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
					pthread_mutex_unlock(&updateDrawMutex);
					#endif
					continue;
				}
			}
			#if defined( AVDL_DIRECT3D11 )
			#elif defined( AVDL_WINDOWS )
			ReleaseMutex(updateDrawMutex);
			#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 ) || defined( AVDL_LINUX )
			pthread_mutex_unlock(&updateDrawMutex);
			#endif
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

void avdl_assetManager_SetCustomAssetLocation(struct avdl_string *newlocation) {
	avdl_string_empty(&avdl_custom_asset_location);
	avdl_string_cat(&avdl_custom_asset_location, avdl_string_toCharPtr(newlocation));
}
