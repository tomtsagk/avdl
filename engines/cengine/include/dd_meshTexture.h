#ifndef DD_MESHTEXTURE_D
#define DD_MESHTEXTURE_D

#include "dd_meshColour.h"
#include "dd_image.h"
#include "dd_opengl.h"

struct dd_meshTexture {
	struct dd_meshColour parent;

	// texture to be used
	struct dd_image *img;

	// texture coordinates
	int dirtyTextures;
	float *t;

	void (*load)(struct dd_mesh *m, const char *filename);
	void (*set_primitive_texcoords)(struct dd_meshTexture *m, float offsetX, float offsetY, float sizeX, float sizeY);
	void (*setTexture)(struct dd_meshTexture *o, struct dd_image *img);
};

// constructor
void dd_meshTexture_create(struct dd_meshTexture *);
void dd_meshTexture_load(struct dd_meshTexture *m, const char *filename);
void dd_meshTexture_set_primitive(struct dd_meshTexture *m, enum dd_primitives shape);
void dd_meshTexture_set_primitive_texcoords(struct dd_meshTexture *m, float offsetX, float offsetY, float sizeX, float sizeY);

void dd_meshTexture_draw(struct dd_meshTexture *m);
void dd_meshTexture_clean(struct dd_meshTexture *m);

void dd_meshTexture_copy(struct dd_meshTexture *dest, struct dd_meshTexture *src);
void dd_meshTexture_setTexture(struct dd_meshTexture *o, struct dd_image *tex);

void dd_meshTexture_combine(struct dd_meshTexture *dst, struct dd_meshTexture *src, float offsetX, float offsetY, float offsetZ);

#endif
