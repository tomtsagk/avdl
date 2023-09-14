#ifndef DD_MESHCOLOUR_H
#define DD_MESHCOLOUR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_mesh.h"

struct dd_vertex_col {
	float pos[3];
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	float col[4];
#else
	float col[3];
#endif
};

struct dd_meshColour {
	struct dd_mesh parent;
	int dirtyColours;
	float *c;
	struct dd_vertex_col *verticesCol;
	int dirtyColourArrayObject;
	void (*set_colour)(struct dd_mesh *m, float r, float g, float b);
	avdl_graphics_mesh *vertexBuffer;
};

// constructor
void dd_meshColour_create(struct dd_meshColour *);

/* Free and Draw functions */
void dd_meshColour_clean(struct dd_meshColour *m);
void dd_meshColour_draw(struct dd_meshColour *m);

// functions to give the mesh its shape
void dd_meshColour_set_primitive(struct dd_meshColour *m, enum dd_primitives shape);
void dd_meshColour_load(struct dd_meshColour *m, const char *filename, int type);

void dd_meshColour_copy(struct dd_meshColour *dest, struct dd_meshColour *src);

void dd_meshColour_set_colour(struct dd_meshColour *m, float r, float g, float b);

void dd_meshColour_combine(struct dd_meshColour *dest, struct dd_meshColour *src, float offsetX, float offsetY, float offsetZ);

#ifdef __cplusplus
}
#endif

#endif /* MESHCOLOUR_H */
