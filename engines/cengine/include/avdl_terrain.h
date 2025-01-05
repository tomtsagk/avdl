#ifndef AVDL_TERRAIN_H
#define AVDL_TERRAIN_H

#include "avdl_mesh.h"
#include "dd_image.h"

struct avdl_terrain {

	// the terrain's mesh
	struct avdl_mesh mesh;

	// the terrain's image
	struct dd_image img;

	int width;
	int height;
	float *heights;
	int loaded;

	// z scale
	float scaleZ;

	// constructor/destructor
	void (*create)(struct avdl_terrain *);
	void (*clean)(struct avdl_terrain *);

	// load terrain texture
	void (*load)(struct avdl_terrain *o, const char *filename);

	void (*draw)(struct avdl_terrain *o);

	int (*isOnTerrain)(struct avdl_terrain *o, float x, float z);
	float (*getSpot)(struct avdl_terrain *o, float x, float z);

	int (*getWidth)(struct avdl_terrain *o);
	int (*getHeight)(struct avdl_terrain *o);
	int (*isLoaded)(struct avdl_terrain *o);

	int (*setScaleZ)(struct avdl_terrain *o, float scale);
};

void avdl_terrain_create(struct avdl_terrain *o);
void avdl_terrain_clean(struct avdl_terrain *o);
void avdl_terrain_draw(struct avdl_terrain *o);

void avdl_terrain_load(struct avdl_terrain *o, const char *filename);
void avdl_terrain_loadLocal(struct avdl_terrain *o, const char *filename);

float avdl_terrain_getSpot(struct avdl_terrain *o, float x, float z);
int avdl_terrain_isOnTerrain(struct avdl_terrain *o, float x, float z);
int avdl_terrain_getWidth(struct avdl_terrain *o);
int avdl_terrain_getHeight(struct avdl_terrain *o);
int avdl_terrain_isLoaded(struct avdl_terrain *o);

int avdl_terrain_setScaleZ(struct avdl_terrain *o, float scale);

#endif
