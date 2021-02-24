#include "dd_meshTexture.h"
#include "dd_filetomesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dd_log.h"

#include "dd_opengl.h"

extern GLuint defaultProgram;

void dd_meshTexture_create(struct dd_meshTexture *m) {
	dd_meshColour_create(&m->parent);
	m->dirtyTextures = 0;
	m->t = 0;
	m->img.tex = 0;
	m->img.pixels = 0;
	m->dirtyImage = 0;
	m->parent.parent.load = (void (*)(struct dd_mesh *, const char *filename)) dd_meshTexture_load;
	m->preloadTexture = (void (*)(struct dd_mesh *, const char *filename)) dd_meshTexture_preloadTexture;
	m->applyTexture = (int (*)(struct dd_mesh *)) dd_meshTexture_applyTexture;
	m->loadTexture = (void (*)(struct dd_mesh *, const char *filename)) dd_meshTexture_loadTexture;
	m->parent.parent.draw = (void (*)(struct dd_mesh *)) dd_meshTexture_draw;
	m->parent.parent.clean = (void (*)(struct dd_mesh *)) dd_meshTexture_clean;
	m->parent.parent.set_primitive = (void (*)(struct dd_mesh *, enum dd_primitives shape)) dd_meshTexture_set_primitive;
	m->set_primitive_texcoords = dd_meshTexture_set_primitive_texcoords;
	m->parent.parent.copy = (void (*)(struct dd_mesh *, struct dd_mesh *))  dd_meshTexture_copy;
	m->copyTexture = (void (*)(struct dd_meshTexture *, struct dd_meshTexture *)) dd_meshTexture_copyTexture;
}

void dd_meshTexture_load(struct dd_meshTexture *m, const char *filename) {
	#if DD_PLATFORM_ANDROID
	#else
	dd_meshTexture_clean(m);
	struct dd_loaded_mesh lm;
	dd_filetomesh(&lm, filename, DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR | DD_FILETOMESH_SETTINGS_TEX_COORD, DD_PLY);
	m->parent.parent.vcount = lm.vcount;
	m->parent.parent.v = lm.v;
	m->parent.parent.dirtyVertices = 1;
	m->parent.c = lm.c;
	m->parent.dirtyColours = 1;
	m->t = lm.t;
	m->dirtyTextures = 1;
	#endif
}

void dd_meshTexture_clean(struct dd_meshTexture *m) {
	dd_meshColour_clean(&m->parent);
	if (m->t && m->dirtyTextures) {
		free(m->t);
		m->t = 0;
		m->dirtyTextures = 0;
	}

	if (m->dirtyImage) {
		dd_image_free(&m->img);
		m->dirtyImage = 0;
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

void dd_meshTexture_preloadTexture(struct dd_meshTexture *m, const char *filename) {
	#if DD_PLATFORM_ANDROID
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "ReadBitmap", "(Ljava/lang/String;)[Ljava/lang/Object;");
	jstring *parameter = (*jniEnv)->NewStringUTF(jniEnv, filename);
	jobjectArray result = (jstring)(*(*jniEnv)->CallStaticObjectMethod)(jniEnv, clazz, MethodID, parameter);

	if (result) {

		// the first object describes the size of the texture
		const jintArray size  = (*(*jniEnv)->GetObjectArrayElement)(jniEnv, result, 0);
		const jint *sizeValues = (*(*jniEnv)->GetIntArrayElements)(jniEnv, size, 0);

		// the second object describes the pixels
		const jintArray pixels  = (*(*jniEnv)->GetObjectArrayElement)(jniEnv, result, 1);
		const jint *pixelValues = (*(*jniEnv)->GetIntArrayElements)(jniEnv, pixels, 0);

		m->img.width = sizeValues[0];
		m->img.height = sizeValues[1];
		m->img.pixelsb = malloc(sizeof(GLubyte) *m->img.width *m->img.height *3);

		/*
		 * read pixels into a new array
		 * for some reason the texture returned is flipped on the y axis
		 * so it can be parsed in reverse, until it's more clear why this
		 * happens
		 */
		jsize len = (*jniEnv)->GetArrayLength(jniEnv, pixels);
		for (int x = 0; x < m->img.width; x++) {
		for (int y = 0; y < m->img.height; y++) {
			int index = ((y *m->img.width) +x);
			int indexReverse = (((m->img.height -(y+1)) *m->img.width) +x);
			m->img.pixelsb[indexReverse*3 +0] = (pixelValues[index] & 0x00FF0000) >> 16;
			m->img.pixelsb[indexReverse*3 +1] = (pixelValues[index] & 0x0000FF00) >>  8;
			m->img.pixelsb[indexReverse*3 +2] = (pixelValues[index] & 0x000000FF);
		}
		}

		(*jniEnv)->ReleaseIntArrayElements(jniEnv, size, sizeValues, JNI_ABORT);
		(*jniEnv)->ReleaseIntArrayElements(jniEnv, pixels, pixelValues, JNI_ABORT);
	}
	#else
	dd_image_load_bmp(&m->img, filename);
	#endif
}

int dd_meshTexture_applyTexture(struct dd_meshTexture *m) {
	#if DD_PLATFORM_ANDROID
	if (m->img.pixelsb) {
	#elif DD_PLATFORM_NATIVE
	if (m->img.pixels) {
	#endif
		dd_image_to_opengl(&m->img);
		m->dirtyImage = 1;
		return 1;
	}
	return 0;
}

void dd_meshTexture_loadTexture(struct dd_meshTexture *m, const char *filename) {
	dd_meshTexture_preloadTexture(m, filename);
	dd_meshTexture_applyTexture(m);
}

void dd_meshTexture_draw(struct dd_meshTexture *m) {

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

	if (m->img.tex) {
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m->img.tex);
		//glUniformi("image", 0);
	}

	GLuint MatrixID = glGetUniformLocation(defaultProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glDrawArrays(GL_TRIANGLES, 0, m->parent.parent.vcount);

	if (m->img.tex) glBindTexture(GL_TEXTURE_2D, 0);
	if (m->t) glDisableVertexAttribArray(2);
	if (m->parent.c) glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void dd_meshTexture_copy(struct dd_meshTexture *dest, struct dd_meshTexture *src) {
	dd_meshColour_copy((struct dd_meshColour *) dest, (struct dd_meshColour *) src);
	if (src->t) {
		dest->t = malloc(sizeof(float) *(dest->parent.parent.vcount*2));
		memcpy(dest->t, src->t, sizeof(float) *(dest->parent.parent.vcount*2));
		dest->dirtyTextures = 1;
	}
}

void dd_meshTexture_copyTexture(struct dd_meshTexture *dest, struct dd_meshTexture *src) {
	if (src->img.tex) {
		dest->img.tex = src->img.tex;
		dest->dirtyImage = 1;
	}
}
