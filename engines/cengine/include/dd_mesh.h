#ifndef DD_MESH_H
#define DD_MESH_H

enum dd_primitives {
	DD_PRIMITIVE_TRIANGLE,
	DD_PRIMITIVE_RECTANGLE,
	DD_PRIMITIVE_BOX,
};

#include "dd_opengl.h"
#include "dd_matrix.h"

#if DD_PLATFORM_ANDROID
#include <jni.h>
extern JavaVM* jvm;
extern JNIEnv *jniEnv;
extern jclass *clazz;
#endif

/* Generic mesh that holds all the data needed to draw a mesh
 * like colors or normals
 * vcount : number of vertices
 * v : array of pointers to vertex data
 * c : array of pointers to vertex colours

 * A mesh must be initialized before it can be drawn. The only proper
 * way to do that is with "filetomesh()" (from "filetomesh.h")
 */
struct dd_mesh {
	int vcount;
	int dirtyVertices;
	float *v;

	void (*draw)(struct dd_mesh *);
	void (*clean)(struct dd_mesh *);
	void (*set_primitive)(struct dd_mesh *m, enum dd_primitives shape);
	void (*load)(struct dd_mesh *m, const char *filename);
	void (*copy)(struct dd_mesh *, struct dd_mesh *);
};

// constructor
void dd_mesh_create(struct dd_mesh *);

/* Free and Draw functions */
void dd_mesh_clean(struct dd_mesh *m);
void dd_mesh_draw(struct dd_mesh *m);

// functions to give the mesh its shape
void dd_mesh_set_primitive(struct dd_mesh *m, enum dd_primitives shape);
void dd_mesh_load(struct dd_mesh *m, const char *filename);

void dd_mesh_copy(struct dd_mesh *dest, struct dd_mesh *src);

#endif /* MESH_H */
