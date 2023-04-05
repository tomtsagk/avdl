#ifndef AVDL_ASSETMANAGER_H
#define AVDL_ASSETMANAGER_H

#include "dd_meshColour.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GAME_ASSET_PREFIX
#define GAME_ASSET_PREFIX ""
#endif

#define AVDL_ASSETMANAGER_MESH 1
#define AVDL_ASSETMANAGER_MESHCOLOUR 2
#define AVDL_ASSETMANAGER_MESHTEXTURE 3
#define AVDL_ASSETMANAGER_TEXTURE 4

struct dd_meshToLoad {
	void *object;
	int meshType;
	char filename[400];
	wchar_t filenameW[400];
	int type;
};

extern struct dd_dynamic_array meshesToLoad;

// init-clean
void avdl_assetManager_init();
void avdl_assetManager_deinit();

// add assets to load
void avdl_assetManager_add(void *object, int meshType, const char *assetname, int type);
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

#ifdef __cplusplus
}
#endif

#endif
