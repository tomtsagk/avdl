#include "dd_meshTexture.h"
#include "dd_filetomesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dd_log.h"
#include "avdl_assetManager.h"
#include "dd_opengl.h"
#include "dd_log.h"
#include <stdlib.h>

extern GLuint defaultProgram;
extern GLuint currentProgram;

void dd_meshTexture_create(struct dd_meshTexture *m) {
	dd_meshColour_create(&m->parent);
	m->dirtyTextures = 0;
	m->t = 0;
	m->img = 0;
	m->hasTransparency = 0;
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

	if (m->hasTransparency) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, m->parent.parent.v);

	if (m->parent.c) {
		glEnableVertexAttribArray(1);
		#if DD_PLATFORM_ANDROID
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, m->parent.c);
		#else
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, m->parent.c);
		#endif
	}

	if (m->t) {
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, m->t);
	}

	if (m->img) {
		m->img->bind(m->img);
	}

	GLint MatrixID = glGetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		dd_log("avdl: dd_meshTexture_draw: location of `matrix` not found in current program");
	}
	else {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());
	}

	glDrawArrays(GL_TRIANGLES, 0, m->parent.parent.vcount);

	if (m->img) {
		m->img->unbind(m->img);
	}
	if (m->t) glDisableVertexAttribArray(2);
	if (m->parent.c) glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	if (m->hasTransparency) {
		glDisable(GL_BLEND);
	}
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
