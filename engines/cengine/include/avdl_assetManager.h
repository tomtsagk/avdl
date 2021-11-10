#ifndef AVDL_ASSETMANAGER_H
#define AVDL_ASSETMANAGER_H

#include "dd_meshColour.h"

#define AVDL_ASSETMANAGER_MESH 1
#define AVDL_ASSETMANAGER_MESHCOLOUR 2
#define AVDL_ASSETMANAGER_MESHTEXTURE 3
#define AVDL_ASSETMANAGER_TEXTURE 4

struct dd_meshToLoad {
	struct dd_meshColour *mesh;
	int meshType;
	char filename[200];
};

extern struct dd_dynamic_array meshesToLoad;

// init-clean
void avdl_assetManager_init();
void avdl_assetManager_clean();

// add assets to load
void avdl_assetManager_add(void *object, int meshType, const char *assetname);
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

#endif
