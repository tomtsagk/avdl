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
	m->openglContextId = -1;

	m->draw = dd_mesh_draw;
	m->clean = dd_mesh_clean;
	m->set_primitive = dd_mesh_set_primitive;
	m->load = dd_mesh_load;
	m->copy = dd_mesh_copy;

	#if !defined( AVDL_DIRECT3D11 )
	m->buffer = 0;
	m->array = 0;
	#endif

	m->vertexBuffer = 0;
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

	#if !defined( AVDL_DIRECT3D11 )
	if (m->array) {
		glDeleteVertexArrays(1, &m->array);
		glDeleteBuffers(1, &m->buffer);
		m->array = 0;
		m->buffer = 0;
	}
	#endif
}

/* draw the mesh itself
 */
void dd_mesh_draw(struct dd_mesh *m) {
	#ifdef AVDL_DIRECT3D11
	if (!m->vertexBuffer && m->v) {
		avdl_graphics_direct3d11_setVertexBufferMesh(m);
	}
	avdl_graphics_direct3d11_drawMeshMesh(m, dd_matrix_globalGet());
	#else

	if (m->array == 0 || m->openglContextId != avdl_graphics_getContextId()) {

                m->openglContextId = avdl_graphics_getContextId();

		GL(glGenVertexArrays(1, &m->array));
		GL(glBindVertexArray(m->array));
	
		GL(glGenBuffers(1, &m->buffer));
		GL(glBindBuffer(GL_ARRAY_BUFFER, m->buffer));
	
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) *m->vcount *3, m->v, GL_STATIC_DRAW));
	
		int pos = glGetAttribLocation(currentProgram, "position");
		GL(glVertexAttribPointer(pos, 3, GL_FLOAT, 0, 0, 0));
		GL(glEnableVertexAttribArray(pos));

	}

	GL(glBindVertexArray(m->array));

	#if defined(AVDL_QUEST2)
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_mesh: location of `matrix` not found in current program");
	}
	else {
		GL(glUniformMatrix4fv(
			MatrixID,
			1,
			GL_TRUE,
			(float *)dd_matrix_globalGet()
		));
	}
	#else
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshColour: location of `matrix` not found in current program");
	}
	else {
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
	}
	#endif
	GL(glDrawArrays(GL_TRIANGLES, 0, m->vcount));
	GL(glBindVertexArray(0));
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
	dd_mesh_clean(dest);
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

void dd_mesh_translatef(struct dd_mesh *o, float x, float y, float z) {
	if (o->v && !o->dirtyVertices) {
		float *p = malloc(sizeof(float) *o->vcount *3);
		memcpy(p, o->v, sizeof(float) *o->vcount *3);
		o->v = p;
		o->dirtyVertices = 1;
	}
	for (int i = 0; i < o->vcount; i++) {
		o->v[i*3 +0] += x;
		o->v[i*3 +1] += y;
		o->v[i*3 +2] += z;
	}
}

void dd_mesh_scalef(struct dd_mesh *o, float x, float y, float z) {
	if (o->v && !o->dirtyVertices) {
		float *p = malloc(sizeof(float) *o->vcount *3);
		memcpy(p, o->v, sizeof(float) *o->vcount *3);
		o->v = p;
		o->dirtyVertices = 1;
	}
	for (int i = 0; i < o->vcount; i++) {
		o->v[i*3 +0] *= x;
		o->v[i*3 +1] *= y;
		o->v[i*3 +2] *= z;
	}
}
