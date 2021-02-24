#include <stdio.h>
#include <stdlib.h>
#include "dd_meshColour.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "dd_matrix.h"

extern GLuint defaultProgram;
extern GLuint risingProgram;

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

	GLuint MatrixID = glGetUniformLocation(defaultProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glDrawArrays(GL_TRIANGLES, 0, m->parent.vcount);

	if (m->c) {
		glDisableVertexAttribArray(1);
	}
	glDisableVertexAttribArray(0);
}

#if DD_PLATFORM_ANDROID
/*
 * load the mesh based on a string instead of a file,
 * used for specific platforms like Android
 */
void dd_meshColour_load(struct dd_meshColour *m, const char *asset) {

	// clean the mesh, if was dirty
	dd_meshColour_clean(m);

	// get string from asset (in java)
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "ReadPly", "(Ljava/lang/String;I)[Ljava/lang/Object;");
	jstring *parameter = (*jniEnv)->NewStringUTF(jniEnv, asset);
	jint parameterSettings = DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR;
	jobjectArray result = (jstring)(*(*jniEnv)->CallStaticObjectMethod)(jniEnv, clazz, MethodID, parameter, &parameterSettings);
	/*
	if ((*jniEnv)->ExceptionCheck(jniEnv) == JNI_TRUE) {
		dd_log("EXCEPTION THROWN");
		(*jniEnv)->ExceptionDescribe(jniEnv);
		(*jniEnv)->ExceptionClear(jniEnv);
	}
	*/

	/*
	 * Reading the asset was successfull,
	 * load the asset with it.
	 */
	if (result) {

		// the first object describes the size of the texture
		const jintArray pos  = (*(*jniEnv)->GetObjectArrayElement)(jniEnv, result, 0);
		const jfloat *posValues = (*(*jniEnv)->GetFloatArrayElements)(jniEnv, pos, 0);

		const jintArray col  = (*(*jniEnv)->GetObjectArrayElement)(jniEnv, result, 1);
		const jint *colValues = (*(*jniEnv)->GetIntArrayElements)(jniEnv, col, 0);

		jsize len = (*jniEnv)->GetArrayLength(jniEnv, pos) /3;
		m->parent.vcount = len;
		m->parent.v = malloc(sizeof(float) *len *3);
		m->c = malloc(sizeof(float) *len *4);
		for (int i = 0; i < len; i++) {
			m->parent.v[i*3+0] = posValues[i*3+0];
			m->parent.v[i*3+1] = posValues[i*3+1];
			m->parent.v[i*3+2] = posValues[i*3+2];
			m->c[i*4+0] = (float) (colValues[i*3+0] /255.0);
			m->c[i*4+1] = (float) (colValues[i*3+1] /255.0);
			m->c[i*4+2] = (float) (colValues[i*3+2] /255.0);
			m->c[i*4+3] = 1;
		}
		m->parent.dirtyVertices = 1;
		m->dirtyColours = 1;

		(*jniEnv)->ReleaseFloatArrayElements(jniEnv, pos, posValues, JNI_ABORT);
		(*jniEnv)->ReleaseIntArrayElements(jniEnv, col, colValues, JNI_ABORT);

		/*
		// get asset string
		const char *returnedValue = (*(*jniEnv)->GetStringUTFChars)(jniEnv, result, 0);

		// load from string to mesh
		struct dd_loaded_mesh lm;
		dd_filetomesh(&lm, returnedValue, DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR, DD_PLY);
		m->parent.vcount = lm.vcount;
		m->parent.v = lm.v;
		m->parent.dirtyVertices = 1;
		m->c = lm.c;
		m->dirtyColours = 1;

		// release gotten string
		(*jniEnv)->ReleaseStringUTFChars(jniEnv, result, returnedValue);
		*/
	}
}
#else
/* load vertex positions and colours from file
 */
void dd_meshColour_load(struct dd_meshColour *m, const char *filename) {
	dd_meshColour_clean(m);
	struct dd_loaded_mesh lm;
	dd_filetomesh(&lm, filename, DD_FILETOMESH_SETTINGS_POSITION | DD_FILETOMESH_SETTINGS_COLOUR, DD_PLY);
	m->parent.vcount = lm.vcount;
	m->parent.v = lm.v;
	m->parent.dirtyVertices = 1;
	m->c = lm.c;
	m->dirtyColours = 1;
}
#endif

void dd_meshColour_copy(struct dd_meshColour *dest, struct dd_meshColour *src) {
	dd_mesh_copy((struct dd_mesh *) dest, (struct dd_mesh *) src);
	if (src->c) {
		dest->c = malloc(sizeof(float) *dest->parent.vcount *4);
		memcpy(dest->c, src->c, sizeof(float) *dest->parent.vcount *4);
		dest->dirtyColours = 1;
	}
}
