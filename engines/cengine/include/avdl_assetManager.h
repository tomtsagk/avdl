#ifndef AVDL_ASSETMANAGER_H
#define AVDL_ASSETMANAGER_H

#include "shared/avdl_string.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GAME_ASSET_PREFIX
#define GAME_ASSET_PREFIX ""
#endif

#define AVDL_ASSETMANAGER_MESH 1
#define AVDL_ASSETMANAGER_MESHCOLOUR 2
#define AVDL_ASSETMANAGER_MESHTEXTURE 3
#define AVDL_ASSETMANAGER_MESH2 5

struct dd_meshToLoad {
	void *object;
	int meshType;
	char filename[400];
	#ifdef AVDL_DIRECT3D11
	char filenameW[400];
	#else
	wchar_t filenameW[400];
	#endif
	void *(*loadOperation)(const char *);
	int (*callback)(void *obj, void *data);
};

extern struct avdl_dynamic_array meshesToLoad;

// init-clean
void avdl_assetManager_init();
void avdl_assetManager_deinit();

// add assets to load
int avdl_assetManager_AddLoadOperation(void *object, const char *assetname, void *(*loadOperation)(const char *), int (*callback)(void *obj, void *data));
int avdl_assetManager_AddLoadOperationLocal(void *object, const char *assetname, void *(*loadOperation)(const char *), int (*callback)(void *obj, void *data));
int avdl_assetManager_add(void *object, int meshType, const char *assetname, int (*callback)(void *obj, void *data));
int avdl_assetManager_addLocal(void *object, int meshType, const char *assetname, int (*callback)(void *obj, void *data));
void avdl_assetManager_remove(int index);
void avdl_assetManager_loadAll();

// getters
int avdl_assetManager_isLoading();
int avdl_assetManager_hasAssetsToLoad();
int avdl_assetManager_isReady();
float avdl_assetManager_getLoadedProportion();

void avdl_assetManager_lockLoading();
void avdl_assetManager_unlockLoading();
void avdl_assetManager_clear();

void avdl_assetManager_setPercentage(float percentage);

// optional custom project location
void avdl_assetManager_SetCustomAssetLocation(struct avdl_string *newlocation);

#ifdef __cplusplus
}
#endif

#endif
