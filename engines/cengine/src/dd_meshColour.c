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
#else
extern void avdl_graphics_direct3d11_drawMesh(struct dd_meshColor *m, struct dd_matrix *);
#endif

// constructor
void dd_meshColour_create(struct dd_meshColour *m) {
	dd_mesh_create(&m->parent);
	m->dirtyColours = 0;
	m->c = 0;
	m->verticesCol = 0;
	m->dirtyColourArrayObject = 0;
	m->parent.set_primitive = (void (*)(struct dd_mesh *, enum dd_primitives)) dd_meshColour_set_primitive;
	m->parent.clean = (void (*)(struct dd_mesh *)) dd_meshColour_clean;
	m->parent.draw = (void (*)(struct dd_mesh *)) dd_meshColour_draw;
	m->parent.load = (void (*)(struct dd_mesh *, const char *filename, int type)) dd_meshColour_load;
	m->set_colour = (void (*)(struct dd_mesh *, float r, float g, float b)) dd_meshColour_set_colour;
	m->parent.copy = (void (*)(struct dd_mesh *, struct dd_mesh *)) dd_meshColour_copy;
	m->parent.combine = (void (*)(struct dd_mesh *, struct dd_mesh *, float x, float y, float z)) dd_meshColour_combine;

	m->vertexBuffer = 0;
}

void dd_meshColour_set_primitive(struct dd_meshColour *m, enum dd_primitives shape) {
	dd_mesh_set_primitive(&m->parent, shape);
}

void dd_meshColour_set_colour(struct dd_meshColour *m, float r, float g, float b) {
	if (m->c) {
		free(m->c);
		m->c = 0;
	}
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	m->c = malloc(m->parent.vcount *sizeof(float) *4);
	m->dirtyColours = 1;
	for (int i = 0; i < m->parent.vcount *4; i += 4) {
		m->c[i+0] = r;
		m->c[i+1] = g;
		m->c[i+2] = b;
		m->c[i+3] = 0;
	}
	#else
	m->c = malloc(m->parent.vcount *sizeof(float) *3);
	m->dirtyColours = 1;
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

	if (m->dirtyColourArrayObject) {
		free(m->verticesCol);
		m->verticesCol = 0;
		m->dirtyColourArrayObject = 0;
	}
}

/* draw the mesh itself */
void dd_meshColour_draw(struct dd_meshColour *m) {

	#ifdef AVDL_DIRECT3D11
	if (!m->vertexBuffer && m->c) {
		avdl_graphics_direct3d11_setVertexBuffer(m);
	}
	avdl_graphics_direct3d11_drawMesh(m, dd_matrix_globalGet());
	#else

	if (m->parent.array == 0 || m->parent.openglContextId != avdl_graphics_getContextId()) {

                m->parent.openglContextId = avdl_graphics_getContextId();

		m->verticesCol = malloc( sizeof(struct dd_vertex_col) *m->parent.vcount );
		m->dirtyColourArrayObject = 1;
		for (int i = 0; i < m->parent.vcount; i++) {
			m->verticesCol[i].pos[0] = m->parent.v[i *3 +0];
			m->verticesCol[i].pos[1] = m->parent.v[i *3 +1];
			m->verticesCol[i].pos[2] = m->parent.v[i *3 +2];
			#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			if (m->c) {
				m->verticesCol[i].col[0] = m->c[i *4 +0];
				m->verticesCol[i].col[1] = m->c[i *4 +1];
				m->verticesCol[i].col[2] = m->c[i *4 +2];
				m->verticesCol[i].col[3] = m->c[i *4 +3];
			}
			else {
				m->verticesCol[i].col[0] = 0;
				m->verticesCol[i].col[1] = 0;
				m->verticesCol[i].col[2] = 0;
				m->verticesCol[i].col[3] = 0;
			}
			#else
			if (m->c) {
				m->verticesCol[i].col[0] = m->c[i *3 +0];
				m->verticesCol[i].col[1] = m->c[i *3 +1];
				m->verticesCol[i].col[2] = m->c[i *3 +2];
			}
			else {
				m->verticesCol[i].col[0] = 0;
				m->verticesCol[i].col[1] = 0;
				m->verticesCol[i].col[2] = 0;
			}
			#endif
		}

		glGenVertexArrays(1, &m->parent.array);
		glBindVertexArray(m->parent.array);
	
		glGenBuffers(1, &m->parent.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m->parent.buffer);
	
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct dd_vertex_col) *m->parent.vcount, m->verticesCol, GL_STATIC_DRAW);
	
		int pos = glGetAttribLocation(currentProgram, "position");
		glVertexAttribPointer(pos, 3, GL_FLOAT, 0, sizeof(struct dd_vertex_col), (void *)offsetof(struct dd_vertex_col, pos));
		glEnableVertexAttribArray(pos);

		int col = glGetAttribLocation(currentProgram, "colour");
		glVertexAttribPointer(col, 3, GL_FLOAT, 0, sizeof(struct dd_vertex_col), (void *)offsetof(struct dd_vertex_col, col));
		glEnableVertexAttribArray(col);
	}

	glBindVertexArray(m->parent.array);
	#if defined(AVDL_QUEST2)
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshColour: location of `matrix` not found in current program");
	}
	else {
		glUniformMatrix4fv(
			MatrixID,
			1,
			GL_TRUE,
			(float *)dd_matrix_globalGet()
		);
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
	glDrawArrays(GL_TRIANGLES, 0, m->parent.vcount);
	glBindVertexArray(0);

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
	dd_meshColour_clean(dest);
	dd_mesh_copy((struct dd_mesh *) dest, (struct dd_mesh *) src);
	if (src->c) {
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		dest->c = malloc(sizeof(float) *src->parent.vcount *4);
		memcpy(dest->c, src->c, sizeof(float) *src->parent.vcount *4);
		#else
		dest->c = malloc(sizeof(float) *src->parent.vcount *3);
		memcpy(dest->c, src->c, sizeof(float) *src->parent.vcount *3);
		#endif
		dest->dirtyColours = 1;
	}
}

void dd_meshColour_combine(struct dd_meshColour *dst, struct dd_meshColour *src, float offsetX, float offsetY, float offsetZ) {
	dd_mesh_combine(dst, src, offsetX, offsetY, offsetZ);

	if ((!dst->c && src->c) || dst->c) {
		dst->c = realloc(dst->c, dst->parent.vcount *sizeof(float) *4);
		dst->dirtyColours = 1;
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
