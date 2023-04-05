#include <stdio.h>
#include <stdlib.h>
#include "dd_mesh.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "avdl_assetManager.h"
#include "dd_log.h"
#include <stdlib.h>
#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

float shape_triangle[] = {
	0, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
};

float shape_rectangle[] = {
	-0.5, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,

	0.5, -0.5, 0,
	0.5, 0.5, 0,
	-0.5, 0.5, 0,
};

float shape_box[] = {
	// front side
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,

	// back side
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,

	// top side
	-0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,
	 0.5,  0.5, -0.5,

	 0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,
	 0.5,  0.5,  0.5,
};

// constructor
void dd_mesh_create(struct dd_mesh *m) {
	m->vcount = 0;
	m->v = 0;
	m->dirtyVertices = 0;

	m->draw = dd_mesh_draw;
	m->clean = dd_mesh_clean;
	m->set_primitive = dd_mesh_set_primitive;
	m->load = dd_mesh_load;
	m->copy = dd_mesh_copy;
}

void dd_mesh_set_primitive(struct dd_mesh *m, enum dd_primitives shape) {

	// set mesh shape based on given value
	switch (shape) {
		case DD_PRIMITIVE_TRIANGLE:
			m->v = shape_triangle;
			m->vcount = sizeof(shape_triangle) /sizeof(float) /3;
			break;

		case DD_PRIMITIVE_RECTANGLE:
			m->v = shape_rectangle;
			m->vcount = sizeof(shape_rectangle) /sizeof(float) /3;
			break;

		case DD_PRIMITIVE_BOX:
			m->v = shape_box;
			m->vcount = sizeof(shape_box) /sizeof(float) /3;
			break;
	}
}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void dd_mesh_clean(struct dd_mesh *m) {
	if (m->v && m->dirtyVertices) {
		free(m->v);
		m->v = 0;
		m->dirtyVertices = 0;
	}
}

/* draw the mesh itself
 */
void dd_mesh_draw(struct dd_mesh *m) {

	#ifndef AVDL_DIRECT3D11
	avdl_graphics_EnableVertexAttribArray(0);
	avdl_graphics_VertexAttribPointer(0, 3, GL_FLOAT, 0, 0, m->v);

	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshColour: location of `matrix` not found in current program");
	}
	else {
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
	}

	avdl_graphics_DrawArrays(m->vcount);

	avdl_graphics_DisableVertexAttribArray(0);
	#endif
}

/*
 * add the mesh to be loaded from the asset manager
 */
void dd_mesh_load(struct dd_mesh *m, const char *asset, int type) {

	// clean the mesh, if was dirty
	dd_mesh_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESH, asset, type);

}

void dd_mesh_copy(struct dd_mesh *dest, struct dd_mesh *src) {
	dest->vcount = src->vcount;
	dest->v = malloc(src->vcount *sizeof(float) *3);
	memcpy(dest->v, src->v, sizeof(float) *src->vcount *3);
	dest->dirtyVertices = 1;
}

void dd_mesh_combine(struct dd_mesh *dst, struct dd_mesh *src, float offsetX, float offsetY, float offsetZ) {
	dst->v = realloc(dst->v, (dst->vcount +src->vcount) *sizeof(float) *3);
	dst->dirtyVertices = 1;
	for (int i = dst->vcount *3; i < (dst->vcount +src->vcount) *3; i += 3) {
		dst->v[i+0] = src->v[(i+0) -(dst->vcount *3)] +offsetX;
		dst->v[i+1] = src->v[(i+1) -(dst->vcount *3)] +offsetY;
		dst->v[i+2] = src->v[(i+2) -(dst->vcount *3)] +offsetZ;
	}
	dst->vcount += src->vcount;
}
