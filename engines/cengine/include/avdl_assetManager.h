#ifndef AVDL_ASSETMANAGER_H
#define AVDL_ASSETMANAGER_H

#include "dd_meshColour.h"
#include <avdl_string.h>

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
#define AVDL_ASSETMANAGER_MESH2 5

struct avdl_assetManager_texture {
	int pixelFormat;
	float *pixels;
	int width;
	int height;
	struct avdl_string filename;
	avdl_texture_id tex;
	int index;
	int graphicsContextId;
	int uses;
};

struct dd_meshToLoad {
	void *object;
	int meshType;
	char filename[400];
	#ifdef AVDL_DIRECT3D11
	char filenameW[400];
	#else
	wchar_t filenameW[400];
	#endif
	int type;
	int (*callback)(void *obj, void *data);
};

extern struct dd_dynamic_array meshesToLoad;

// init-clean
void avdl_assetManager_init();
void avdl_assetManager_deinit();

// add assets to load
int avdl_assetManager_add(void *object, int meshType, const char *assetname, int type, int (*callback)(void *obj, void *data));
void avdl_assetManager_remove(int index);
void avdl_assetManager_addLocal(void *object, int meshType, const char *assetname, int type);
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

// Textures
void avdl_assetManager_CleanTexture(struct avdl_assetManager_texture *t);

#ifdef __cplusplus
}
#endif

#endif
