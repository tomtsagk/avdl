#include <stdio.h>
#include <stdlib.h>
#include "dd_meshColour.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "dd_matrix.h"
#include "avdl_assetManager.h"
#include "dd_log.h"
#include <stdlib.h>
#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

// constructor
void dd_meshColour_create(struct dd_meshColour *m) {
	dd_mesh_create(&m->parent);
	m->dirtyColours = 0;
	m->c = 0;
	m->parent.set_primitive = (void (*)(struct dd_mesh *, enum dd_primitives)) dd_meshColour_set_primitive;
	m->parent.clean = (void (*)(struct dd_mesh *)) dd_meshColour_clean;
	m->parent.draw = (void (*)(struct dd_mesh *)) dd_meshColour_draw;
	m->parent.load = (void (*)(struct dd_mesh *, const char *filename, int type)) dd_meshColour_load;
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

	#ifndef AVDL_DIRECT3D11
	avdl_graphics_EnableVertexAttribArray(0);
	avdl_graphics_VertexAttribPointer(0, 3, GL_FLOAT, 0, 0, m->parent.v);

	if (m->c) {
		avdl_graphics_EnableVertexAttribArray(1);
		#if DD_PLATFORM_ANDROID
		avdl_graphics_VertexAttribPointer(1, 4, GL_FLOAT, 1, 0, m->c);
		#else
		avdl_graphics_VertexAttribPointer(1, 3, GL_FLOAT, 1, 0, m->c);
		#endif
	}

	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshColour: location of `matrix` not found in current program");
	}
	else {
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
	}

	avdl_graphics_DrawArrays(m->parent.vcount);

	if (m->c) {
		avdl_graphics_DisableVertexAttribArray(1);
	}
	avdl_graphics_DisableVertexAttribArray(0);
	#endif
}

/*
 * load the mesh based on a string instead of a file,
 * used for specific platforms like Android
 */
void dd_meshColour_load(struct dd_meshColour *m, const char *asset, int type) {

	// clean the mesh, if was dirty
	dd_meshColour_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESHCOLOUR, asset, type);
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
