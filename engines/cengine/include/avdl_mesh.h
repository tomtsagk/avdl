#ifndef AVDL_MESH_H
#define AVDL_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

enum avdl_primitives {
	AVDL_PRIMITIVE_TRIANGLE,
	AVDL_PRIMITIVE_RECTANGLE,
	AVDL_PRIMITIVE_BOX,
	AVDL_PRIMITIVE_BOX_FLIP,
	AVDL_PRIMITIVE_LINE,
};

#include "avdl_graphics.h"
#include "dd_matrix.h"
#include "avdl_texture.h"
#include "dd_filetomesh.h"
#include "avdl_vec3.h"

#define TEXTURES_COUNT 5

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

	// bounds
	struct avdl_vec3 boundsCenter;
	struct avdl_vec3 boundsExtend;

	// bump map
	float *tan;
	int dirtyTan;
	float *bitan;
	int dirtyBitan;

	// structs
	void *verticesCol;
	int dirtyColourArrayObject;

	// draw solid or wireframe
	int draw_type;
	float lineWidth;

	// array buffer object
	#if !defined( AVDL_DIRECT3D11 )
	GLuint buffer;
	GLuint array;
	#endif

	// graphics context
	int graphicsContextId;

	// textures
	// texture to be used
	struct avdl_texture *img;
	struct avdl_texture *img_normal;
	struct avdl_texture *img_extra[TEXTURES_COUNT];

	// transparency
	int hasTransparency;

	// init mesh
	void (*clean)(struct avdl_mesh *);

	void (*LoadFromLoadedMesh)(struct avdl_mesh *o, struct dd_loaded_mesh *lm);

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
void avdl_mesh_loadLocal(struct avdl_mesh *m, const char *filename, int type);

void avdl_mesh_copy(struct avdl_mesh *dest, struct avdl_mesh *src);
void avdl_mesh_combine(struct avdl_mesh *dest, struct avdl_mesh *src, float offsetX, float offsetY, float offsetZ);

void avdl_mesh_translatef(struct avdl_mesh *o, float x, float y, float z);
void avdl_mesh_scalef(struct avdl_mesh *o, float x, float y, float z);

void avdl_mesh_set_colour(struct avdl_mesh *m, float r, float g, float b);

void avdl_mesh_setTexture(struct avdl_mesh *o, struct avdl_texture *tex);
void avdl_mesh_setTextureNormal(struct avdl_mesh *o, struct avdl_texture *tex);
void avdl_mesh_setTextureIndex(struct avdl_mesh *o, struct avdl_texture *tex, int index);
void avdl_mesh_setTransparency(struct avdl_mesh *o, int transparency);
void avdl_mesh_set_primitive_texcoords(struct avdl_mesh *m, float offsetX, float offsetY, float sizeX, float sizeY);

void avdl_mesh_setWireframe(struct avdl_mesh *o);
void avdl_mesh_setSolid(struct avdl_mesh *o);
void avdl_mesh_SetTypeLine(struct avdl_mesh *o, float lineWidth);

int avdl_mesh_hasTexture(struct avdl_mesh *o);

void avdl_mesh_LoadFromLoadedMesh(struct avdl_mesh *o, struct dd_loaded_mesh *loadedMesh);

struct avdl_vec3 *avdl_mesh_GetBoundsCenter(struct avdl_mesh *o);
struct avdl_vec3 *avdl_mesh_GetBoundsExtend(struct avdl_mesh *o);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */
