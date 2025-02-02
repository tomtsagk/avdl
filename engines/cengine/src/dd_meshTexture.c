#include "dd_meshTexture.h"
#include "dd_filetomesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_log.h"
#include "avdl_assetManager.h"
#include "dd_game.h"
#include <stdlib.h>
#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

void dd_meshTexture_create(struct dd_meshTexture *m) {
	dd_meshColour_create(&m->parent);
	m->dirtyTextures = 0;
	m->t = 0;
	m->img = 0;
	m->hasTransparency = 0;
	m->verticesTex = 0;
	m->dirtyTextureArrayObject = 0;
	m->parent.parent.load = (void (*)(struct dd_mesh *, const char *filename, int type)) dd_meshTexture_load;
	m->parent.parent.draw = (void (*)(struct dd_mesh *)) dd_meshTexture_draw;
	m->parent.parent.clean = (void (*)(struct dd_mesh *)) dd_meshTexture_clean;
	m->parent.parent.set_primitive = (void (*)(struct dd_mesh *, enum dd_primitives shape)) dd_meshTexture_set_primitive;
	m->set_primitive_texcoords = dd_meshTexture_set_primitive_texcoords;
	m->setTexture = dd_meshTexture_setTexture;
	m->setTransparency = dd_meshTexture_setTransparency;
	m->parent.parent.copy = (void (*)(struct dd_mesh *, struct dd_mesh *))  dd_meshTexture_copy;
	m->parent.parent.combine = (void (*)(struct dd_mesh *, struct dd_mesh *, float x, float y, float z))  dd_meshTexture_combine;
}

void dd_meshTexture_load(struct dd_meshTexture *m, const char *filename, int type) {

	// clean the mesh, if was dirty
	dd_meshTexture_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESHTEXTURE, filename, type);

}

void dd_meshTexture_clean(struct dd_meshTexture *m) {
	dd_meshColour_clean(&m->parent);
	if (m->t && m->dirtyTextures) {
		free(m->t);
		m->t = 0;
		m->dirtyTextures = 0;
	}

	if (m->dirtyTextureArrayObject) {
		free(m->verticesTex);
		m->verticesTex = 0;
		m->dirtyTextureArrayObject = 0;
	}

}

void dd_meshTexture_set_primitive(struct dd_meshTexture *m, enum dd_primitives shape) {
	dd_meshColour_set_primitive((struct dd_meshColour *)m, shape);

	switch (shape) {
		case DD_PRIMITIVE_TRIANGLE:
			break;
		case DD_PRIMITIVE_BOX:
			break;
		case DD_PRIMITIVE_RECTANGLE:
			m->t = malloc(sizeof(float) *6 *2);
			m->dirtyTextures = 1;
			for (int i = 0; i < 6; i++) {
				m->t[i*2+0] = m->parent.parent.v[i*3+0] +0.5;
				m->t[i*2+1] = m->parent.parent.v[i*3+1] +0.5;
			}
			break;
	}
}

void dd_meshTexture_set_primitive_texcoords(struct dd_meshTexture *m, float offsetX, float offsetY, float sizeX, float sizeY) {
	for (int i = 0; i < m->parent.parent.vcount*2; i += 2) {
		m->t[i+0] *= sizeX;
		m->t[i+0] += offsetX;

		m->t[i+1] *= sizeY;
		m->t[i+1] += offsetY;
	}
}

void dd_meshTexture_draw(struct dd_meshTexture *m) {

	#ifdef AVDL_DIRECT3D11
	if (!m->parent.parent.vertexBuffer && m->t) {
		avdl_graphics_direct3d11_setVertexBufferTexture(m);
	}
	avdl_graphics_direct3d11_drawMeshTexture(m, dd_matrix_globalGet());
	#else

	if (m->parent.parent.array == 0 || m->parent.parent.openglContextId != avdl_graphics_getContextId()) {

                m->parent.parent.openglContextId = avdl_graphics_getContextId();

		m->verticesTex = malloc( sizeof(struct dd_vertex_tex) *m->parent.parent.vcount );
		m->dirtyTextureArrayObject = 1;
		for (int i = 0; i < m->parent.parent.vcount; i++) {
			m->verticesTex[i].pos[0] = m->parent.parent.v[i *3 +0];
			m->verticesTex[i].pos[1] = m->parent.parent.v[i *3 +1];
			m->verticesTex[i].pos[2] = m->parent.parent.v[i *3 +2];

			#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			if (m->parent.c) {
				m->verticesTex[i].col[0] = m->parent.c[i *4 +0];
				m->verticesTex[i].col[1] = m->parent.c[i *4 +1];
				m->verticesTex[i].col[2] = m->parent.c[i *4 +2];
				m->verticesTex[i].col[3] = m->parent.c[i *4 +3];
			}
			else {
				m->verticesTex[i].col[0] = 0;
				m->verticesTex[i].col[1] = 0;
				m->verticesTex[i].col[2] = 0;
				m->verticesTex[i].col[3] = 0;
			}
			#else
			if (m->parent.c) {
				m->verticesTex[i].col[0] = m->parent.c[i *3 +0];
				m->verticesTex[i].col[1] = m->parent.c[i *3 +1];
				m->verticesTex[i].col[2] = m->parent.c[i *3 +2];
			}
			else {
				m->verticesTex[i].col[0] = 0;
				m->verticesTex[i].col[1] = 0;
				m->verticesTex[i].col[2] = 0;
			}
			#endif

			if (m->t) {
				m->verticesTex[i].tex[0] = m->t[i *2 +0];
				m->verticesTex[i].tex[1] = m->t[i *2 +1];
			}
			else {
				m->verticesTex[i].tex[0] = 0;
				m->verticesTex[i].tex[1] = 0;
			}
		}

		GL(glGenVertexArrays(1, &m->parent.parent.array));
		GL(glBindVertexArray(m->parent.parent.array));
	
		GL(glGenBuffers(1, &m->parent.parent.buffer));
		GL(glBindBuffer(GL_ARRAY_BUFFER, m->parent.parent.buffer));
	
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(struct dd_vertex_tex) *m->parent.parent.vcount, m->verticesTex, GL_STATIC_DRAW));
	
		int pos = glGetAttribLocation(currentProgram, "position");
		GL(glVertexAttribPointer(pos, 3, GL_FLOAT, 0, sizeof(struct dd_vertex_tex), (void *)offsetof(struct dd_vertex_tex, pos)));
		GL(glEnableVertexAttribArray(pos));

		int col = glGetAttribLocation(currentProgram, "colour");
		if (col != -1) {
			GL(glVertexAttribPointer(col, 3, GL_FLOAT, 0, sizeof(struct dd_vertex_tex), (void *)offsetof(struct dd_vertex_tex, col)));
			GL(glEnableVertexAttribArray(col));
		}

		int tex = glGetAttribLocation(currentProgram, "texCoord");
		if (tex != -1) {
			GL(glVertexAttribPointer(tex, 2, GL_FLOAT, 0, sizeof(struct dd_vertex_tex), (void *)offsetof(struct dd_vertex_tex, tex)));
			GL(glEnableVertexAttribArray(tex));
		}

	}

	if (m->hasTransparency) {
		avdl_graphics_EnableBlend();
	}

	if (m->img) {
		m->img->bind(m->img);
	}

	GL(glBindVertexArray(m->parent.parent.array));

	#if defined(AVDL_QUEST2)
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		//avdl_log("avdl: dd_meshTexture: location of `matrix` not found in current program");
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
		//avdl_log("avdl: dd_meshColour: location of `matrix` not found in current program");
	}
	else {
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
	}
	#endif

	GL(glDrawArrays(GL_TRIANGLES, 0, m->parent.parent.vcount));
	GL(glBindVertexArray(0));

	if (m->img) {
		m->img->unbind(m->img);
	}

	if (m->hasTransparency) {
		avdl_graphics_DisableBlend();
	}

	#endif
}

void dd_meshTexture_copy(struct dd_meshTexture *dest, struct dd_meshTexture *src) {
	dd_meshColour_copy((struct dd_meshColour *) dest, (struct dd_meshColour *) src);
	if (src->t) {
		dest->t = malloc(sizeof(float) *(dest->parent.parent.vcount*2));
		memcpy(dest->t, src->t, sizeof(float) *(dest->parent.parent.vcount*2));
		dest->dirtyTextures = 1;
	}
}

void dd_meshTexture_setTexture(struct dd_meshTexture *o, struct dd_image *tex) {
	o->img = tex;
}

void dd_meshTexture_combine(struct dd_meshTexture *dst, struct dd_meshTexture *src, float offsetX, float offsetY, float offsetZ) {
	dd_meshColour_combine(dst, src, offsetX, offsetY, offsetZ);

	if ((!dst->t && src->t) || dst->t) {
		dst->t = realloc(dst->t, dst->parent.parent.vcount *sizeof(float) *2);
		dst->dirtyTextures = 1;
		int oldVertices = dst->parent.parent.vcount -src->parent.parent.vcount;
		for (int i = oldVertices *2; i < dst->parent.parent.vcount *2; i += 2) {
			if (src->t) {
				dst->t[i+0] = src->t[(i+0) -(oldVertices *2)];
				dst->t[i+1] = src->t[(i+1) -(oldVertices *2)];
			}
			else {
				dst->t[i+0] = 0;
				dst->t[i+1] = 0;
			}
		}
	}
}

void dd_meshTexture_setTransparency(struct dd_meshTexture *o, int transparency) {
	o->hasTransparency = transparency;
}
