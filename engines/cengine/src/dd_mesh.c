#include <stdio.h>
#include <stdlib.h>
#include "dd_mesh.h"
#include "dd_filetomesh.h"
#include <string.h>

extern GLuint defaultProgram;
extern GLuint risingProgram;

#if DD_PLATFORM_ANDROID
jclass *clazz;
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
	MatrixID = glGetUniformLocation(risingProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glDrawArrays(GL_TRIANGLES, 0, m->vcount);

	glDisableVertexAttribArray(0);
}

#if DD_PLATFORM_ANDROID
/*
 * load the mesh based on a string instead of a file,
 * used for specific platforms like Android
 */
void dd_mesh_load(struct dd_mesh *m, const char *asset) {

	// clean the mesh, if was dirty
	dd_mesh_clean(m);

	// get string from asset (in java)
	jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "ReadPly", "(Ljava/lang/String;I)[Ljava/lang/Object;");
	jstring *parameter = (*jniEnv)->NewStringUTF(jniEnv, asset);
	jint parameterSettings = DD_FILETOMESH_SETTINGS_POSITION;
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

		jsize len = (*jniEnv)->GetArrayLength(jniEnv, pos) /3;
		m->vcount = len;
		m->v = malloc(sizeof(float) *len *3);
		for (int i = 0; i < len; i++) {
			m->v[i*3+0] = posValues[i*3+0];
			m->v[i*3+1] = posValues[i*3+1];
			m->v[i*3+2] = posValues[i*3+2];
		}
		m->dirtyVertices = 1;

		(*jniEnv)->ReleaseFloatArrayElements(jniEnv, pos, posValues, JNI_ABORT);

	}

}
#else
void dd_mesh_load(struct dd_mesh *m, const char *filename) {
	dd_mesh_clean(m);
	struct dd_loaded_mesh lm;
	dd_filetomesh(&lm, filename, DD_FILETOMESH_SETTINGS_POSITION, DD_PLY);
	m->vcount = lm.vcount;
	m->v = lm.v;
	m->dirtyVertices = 1;
}
#endif

void dd_mesh_copy(struct dd_mesh *dest, struct dd_mesh *src) {
	dest->vcount = src->vcount;
	dest->v = malloc(src->vcount *sizeof(float) *3);
	memcpy(dest->v, src->v, sizeof(float) *src->vcount *3);
	dest->dirtyVertices = 1;
}
