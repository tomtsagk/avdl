#include <stdio.h>
#include <stdlib.h>
#include "dd_mesh.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "avdl_assetManager.h"

extern GLuint defaultProgram;

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
	if (m->v && m->dirtyVertices) free(m->v);
}

/* draw the mesh itself
 */
void dd_mesh_draw(struct dd_mesh *m) {

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, m->v);

	GLuint MatrixID = glGetUniformLocation(defaultProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glDrawArrays(GL_TRIANGLES, 0, m->vcount);

	glDisableVertexAttribArray(0);
}

/*
 * add the mesh to be loaded from the asset manager
 */
void dd_mesh_load(struct dd_mesh *m, const char *asset) {

	// clean the mesh, if was dirty
	dd_mesh_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESH, asset);

}

void dd_mesh_copy(struct dd_mesh *dest, struct dd_mesh *src) {
	dest->vcount = src->vcount;
	dest->v = malloc(src->vcount *sizeof(float) *3);
	memcpy(dest->v, src->v, sizeof(float) *src->vcount *3);
	dest->dirtyVertices = 1;
}
