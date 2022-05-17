#include <stdio.h>
#include <stdlib.h>
#include "dd_meshColour.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "dd_matrix.h"
#include "avdl_assetManager.h"
#include "dd_log.h"
#include <stdlib.h>

extern GLuint defaultProgram;
extern GLuint currentProgram;

// constructor
void dd_meshColour_create(struct dd_meshColour *m) {
	dd_mesh_create(&m->parent);
	m->dirtyColours = 0;
	m->c = 0;
	m->parent.set_primitive = (void (*)(struct dd_mesh *, enum dd_primitives)) dd_meshColour_set_primitive;
	m->parent.clean = (void (*)(struct dd_mesh *)) dd_meshColour_clean;
	m->parent.draw = (void (*)(struct dd_mesh *)) dd_meshColour_draw;
	m->parent.load = (void (*)(struct dd_mesh *, const char *filename)) dd_meshColour_load;
	m->set_colour = (void (*)(struct dd_mesh *, float r, float g, float b)) dd_meshColour_set_colour;
	m->parent.copy = (void (*)(struct dd_mesh *, struct dd_mesh *)) dd_meshColour_copy;
	m->parent.combine = (void (*)(struct dd_mesh *, struct dd_mesh *, float x, float y, float z)) dd_meshColour_combine;
}

void dd_meshColour_set_primitive(struct dd_meshColour *m, enum dd_primitives shape) {
	dd_mesh_set_primitive(&m->parent, shape);
}

void dd_meshColour_set_colour(struct dd_meshColour *m, float r, float g, float b) {
	if (m->c) {
		free(m->c);
		m->c = 0;
	}
	#if DD_PLATFORM_ANDROID
	m->c = malloc(m->parent.vcount *sizeof(float) *4);
	for (int i = 0; i < m->parent.vcount *4; i += 4) {
		m->c[i+0] = r;
		m->c[i+1] = g;
		m->c[i+2] = b;
		m->c[i+3] = 0;
	}
	#else
	m->c = malloc(m->parent.vcount *sizeof(float) *3);
	for (int i = 0; i < m->parent.vcount *3; i += 3) {
		m->c[i+0] = r;
		m->c[i+1] = g;
		m->c[i+2] = b;
	}
	#endif
}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void dd_meshColour_clean(struct dd_meshColour *m) {
	dd_mesh_clean(&m->parent);
	if (m->c && m->dirtyColours) {
		free(m->c);
		m->c = 0;
		m->dirtyColours = 0;
	}
}

/* draw the mesh itself */
void dd_meshColour_draw(struct dd_meshColour *m) {

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, m->parent.v);

	if (m->c) {
		glEnableVertexAttribArray(1);
		#if DD_PLATFORM_ANDROID
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, m->c);
		#else
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, m->c);
		#endif
	}

	GLint MatrixID = glGetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshTexture_draw: location of `matrix` not found in current program");
	}
	else {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());
	}

	glDrawArrays(GL_TRIANGLES, 0, m->parent.vcount);

	if (m->c) {
		glDisableVertexAttribArray(1);
	}
	glDisableVertexAttribArray(0);
}

/*
 * load the mesh based on a string instead of a file,
 * used for specific platforms like Android
 */
void dd_meshColour_load(struct dd_meshColour *m, const char *asset) {

	// clean the mesh, if was dirty
	dd_meshColour_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESHCOLOUR, asset);
}

void dd_meshColour_copy(struct dd_meshColour *dest, struct dd_meshColour *src) {
	dd_mesh_copy((struct dd_mesh *) dest, (struct dd_mesh *) src);
	if (src->c) {
		dest->c = malloc(sizeof(float) *dest->parent.vcount *4);
		memcpy(dest->c, src->c, sizeof(float) *dest->parent.vcount *4);
		dest->dirtyColours = 1;
	}
}

void dd_meshColour_combine(struct dd_meshColour *dst, struct dd_meshColour *src, float offsetX, float offsetY, float offsetZ) {
	dd_mesh_combine(dst, src, offsetX, offsetY, offsetZ);

	if ((!dst->c && src->c) || dst->c) {
		dst->c = realloc(dst->c, dst->parent.vcount *sizeof(float) *4);
		int oldVertices = dst->parent.vcount -src->parent.vcount;
		for (int i = oldVertices *4; i < dst->parent.vcount *4; i += 4) {
			// get new mesh's colour
			if (src->c) {
				dst->c[i+0] = src->c[(i+0) -(oldVertices *4)];
				dst->c[i+1] = src->c[(i+1) -(oldVertices *4)];
				dst->c[i+2] = src->c[(i+2) -(oldVertices *4)];
				dst->c[i+3] = src->c[(i+3) -(oldVertices *4)];
			}
			// new mesh has no colour - add default
			else {
				dst->c[i+0] = 0;
				dst->c[i+1] = 0;
				dst->c[i+2] = 0;
				dst->c[i+3] = 0;
			}
		}
	}
}
