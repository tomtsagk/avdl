#ifndef AVDL_MESH_H
#define AVDL_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

enum avdl_primitives {
	AVDL_PRIMITIVE_TRIANGLE,
	AVDL_PRIMITIVE_RECTANGLE,
	AVDL_PRIMITIVE_BOX,
};

#include "avdl_graphics.h"
#include "dd_matrix.h"
#include "dd_image.h"

struct avdl_mesh {

	// number of vertices
	int vcount;

	// vertex attributes
	float *v;
	int dirtyVertices;
	float *c;
	int dirtyColours;
	float *t;
	int dirtyTextures;
	float *n;
	int dirtyNormals;

	// structs
	void *verticesCol;
	int dirtyColourArrayObject;

	// draw solid or wireframe
	int draw_type;

	// array buffer object
	#if !defined( AVDL_DIRECT3D11 )
	GLuint buffer;
	GLuint array;
	#endif

	// graphics context
	int graphicsContextId;

	// textures
	// texture to be used
	struct dd_image *img;

	// transparency
	int hasTransparency;

	// init mesh
	void (*set_primitive)(struct avdl_mesh *m, enum avdl_primitives shape);
	void (*load)(struct avdl_mesh *m, const char *filename, int type);

	void (*draw)(struct avdl_mesh *);
	void (*clean)(struct avdl_mesh *);
	void (*copy)(struct avdl_mesh *, struct avdl_mesh *);

	void (*combine)(struct avdl_mesh *dst, struct avdl_mesh *src, float offsetX, float offsetY, float offsetZ);

	void (*translatef)(struct avdl_mesh *, float x, float y, float z);
	void (*scalef)(struct avdl_mesh *, float x, float y, float z);

	void (*set_colour)(struct avdl_mesh *m, float r, float g, float b);

	void (*set_primitive_texcoords)(struct avdl_mesh *m, float offsetX, float offsetY, float sizeX, float sizeY);
	void (*setTexture)(struct avdl_mesh *o, struct dd_image *img);
	void (*setTransparency)(struct avdl_mesh *o, int transparency);

	avdl_graphics_mesh* vertexBuffer;
};

// constructor
void avdl_mesh_create(struct avdl_mesh *);

/* Free and Draw functions */
void avdl_mesh_clean(struct avdl_mesh *m);
void avdl_mesh_draw(struct avdl_mesh *m);

// functions to give the mesh its shape
void avdl_mesh_set_primitive(struct avdl_mesh *m, enum avdl_primitives shape);
void avdl_mesh_load(struct avdl_mesh *m, const char *filename, int type);

void avdl_mesh_copy(struct avdl_mesh *dest, struct avdl_mesh *src);
void avdl_mesh_combine(struct avdl_mesh *dest, struct avdl_mesh *src, float offsetX, float offsetY, float offsetZ);

void avdl_mesh_translatef(struct avdl_mesh *o, float x, float y, float z);
void avdl_mesh_scalef(struct avdl_mesh *o, float x, float y, float z);

void avdl_mesh_set_colour(struct avdl_mesh *m, float r, float g, float b);

void avdl_mesh_setTexture(struct avdl_mesh *o, struct dd_image *tex);
void avdl_mesh_setTransparency(struct avdl_mesh *o, int transparency);
void avdl_mesh_set_primitive_texcoords(struct avdl_mesh *m, float offsetX, float offsetY, float sizeX, float sizeY);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */
