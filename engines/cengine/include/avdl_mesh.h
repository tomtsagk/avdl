#ifndef AVDL_MESH_H
#define AVDL_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

enum avdl_primitives {
	AVDL_PRIMITIVE_TRIANGLE,
	AVDL_PRIMITIVE_TRIANGLE_WIREFRAME,
	AVDL_PRIMITIVE_RECTANGLE,
	AVDL_PRIMITIVE_RECTANGLE_WIREFRAME,
	AVDL_PRIMITIVE_BOX,
	AVDL_PRIMITIVE_BOX_CORNERS,
	AVDL_PRIMITIVE_BOX_FLIP,
	AVDL_PRIMITIVE_LINE,
};

#include "avdl_graphics.h"
#include "dd_matrix.h"
#include "avdl_texture.h"
#include "avdl_vec3.h"

#define TEXTURES_COUNT 5

struct avdl_mesh_data {
	unsigned int vcount;
	enum avdl_graphics_vtype verticesType;

	float *v;
	int dirtyVertices;
	float *c;
	float *t;
	float *n;
	float *tan;
	float *bitan;
	int *boneIds;
	float *weights;
	void *indices;
	enum avdl_graphics_indicetype indicesType;
	unsigned int indicesCount;

	// animations
	int boneCount;
	struct dd_matrix *inverseBindMatrices;
	struct dd_animation *animations;
	int animationsCount;
	int rootIndex;
	int **children_indices;
	int *children_indices_count;
	struct dd_matrix rootMatrix;

	struct avdl_vec3 boundsCenter;
	struct avdl_vec3 boundsExtend;

	avdl_mesh_id id;
	avdl_mesh_id buffer;
	avdl_mesh_id bufferIndices;
	int graphicsContextId;

	struct avdl_string filename;
	int uses;

	int hasError;
};


struct avdl_mesh {

	struct avdl_mesh_data *data;

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

	avdl_graphics_mesh* vertexBuffer;
};

// global data
void avdl_mesh_InitGlobalData();
void avdl_mesh_DeinitGlobalData();

// constructor
void avdl_mesh_create(struct avdl_mesh *);

/* Free and Draw functions */
void avdl_mesh_clean(struct avdl_mesh *m);
void avdl_mesh_draw(struct avdl_mesh *m);

// functions to give the mesh its shape
void avdl_mesh_set_primitive(struct avdl_mesh *m, enum avdl_primitives shape);
void avdl_mesh_load(struct avdl_mesh *m, const char *filename);
void avdl_mesh_loadLocal(struct avdl_mesh *m, const char *filename);

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

//void avdl_mesh_LoadFromLoadedMesh(struct avdl_mesh *o, struct dd_loaded_mesh *loadedMesh);

struct avdl_vec3 *avdl_mesh_GetBoundsCenter(struct avdl_mesh *o);
struct avdl_vec3 *avdl_mesh_GetBoundsExtend(struct avdl_mesh *o);

int avdl_mesh_SetCustomData(struct avdl_mesh *o, int vcount, float *pos, float *col, float *tex);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */
