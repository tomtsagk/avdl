#ifndef DD_MESHTEXTURE_D
#define DD_MESHTEXTURE_D

#include "dd_meshColour.h"
#include "dd_image.h"
#include "dd_opengl.h"

struct dd_meshTexture {
	struct dd_meshColour parent;
	int dirtyImage;
	struct dd_image img;
	int dirtyTextures;
	float *t;
	int openglContextId;
	char *assetName;
	void (*load)(struct dd_mesh *m, const char *filename);
	void (*preloadTexture)(struct dd_mesh *m, const char *filename);
	int (*applyTexture)(struct dd_mesh *m);
	void (*loadTexture)(struct dd_mesh *m, const char *filename);
	void (*set_primitive_texcoords)(struct dd_meshTexture *m, float offsetX, float offsetY, float sizeX, float sizeY);
	void (*copyTexture)(struct dd_meshTexture *dest, struct dd_meshTexture *src);
};

// constructor
void dd_meshTexture_create(struct dd_meshTexture *);
void dd_meshTexture_load(struct dd_meshTexture *m, const char *filename);
void dd_meshTexture_preloadTexture(struct dd_meshTexture *m, char *filename);
int dd_meshTexture_applyTexture(struct dd_meshTexture *m);
void dd_meshTexture_loadTexture(struct dd_meshTexture *m, const char *filename);
void dd_meshTexture_set_primitive(struct dd_meshTexture *m, enum dd_primitives shape);
void dd_meshTexture_set_primitive_texcoords(struct dd_meshTexture *m, float offsetX, float offsetY, float sizeX, float sizeY);

void dd_meshTexture_draw(struct dd_meshTexture *m);
void dd_meshTexture_clean(struct dd_meshTexture *m);

void dd_meshTexture_copy(struct dd_meshTexture *dest, struct dd_meshTexture *src);
void dd_meshTexture_copyTexture(struct dd_meshTexture *dest, struct dd_meshTexture *src);

#endif
