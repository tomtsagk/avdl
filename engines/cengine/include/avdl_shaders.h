#ifndef AVDL_SHADERS_H
#define AVDL_SHADERS_H

#include <dd_opengl.h>

struct avdl_program {
	char *sdrVertexSrc;
	char *sdrFragmentSrc;
	int openglContext;
	int program;

	void (*clean)(struct avdl_program *);
	void (*setVertexShader)(struct avdl_program *, char *source);
	void (*setFragmentShader)(struct avdl_program *, char *source);
	void (*useProgram)(struct avdl_program *);
};

void avdl_program_create(struct avdl_program *);
void avdl_program_clean(struct avdl_program *);
void avdl_program_setVertexShader(struct avdl_program *, char *source);
void avdl_program_setFragmentShader(struct avdl_program *, char *source);
void avdl_program_useProgram(struct avdl_program *);

/*
 * function to change the current program
 */
void avdl_useProgram(struct avdl_program *o);

/*
 * get/edit uniforms
 */
int avdl_getUniformLocation(struct avdl_program *o, char *varname);
#define avdl_setUniformMatrix4f(uniform, matrix) glUniformMatrix4fv(unfirom, 1, GL_FALSE, matrix)

#define avdl_setUniform1f(uniform, f1)             glUniform1f(uniform, f1)
#define avdl_setUniform2f(uniform, f1, f2)         glUniform2f(uniform, f1, f2)
#define avdl_setUniform3f(uniform, f1, f2, f3)     glUniform3f(uniform, f1, f2, f3)
#define avdl_setUniform4f(uniform, f1, f2, f3, f4) glUniform4f(uniform, f1, f2, f3, f4)

#define avdl_setUniform1i(uniform, i1)             glUniform1i(uniform, i1)
#define avdl_setUniform2i(uniform, i1, i2)         glUniform2i(uniform, i1, i2)
#define avdl_setUniform3i(uniform, i1, i2, i3)     glUniform3i(uniform, i1, i2, i3)
#define avdl_setUniform4i(uniform, i1, i2, i3, i4) glUniform4i(uniform, i1, i2, i3, i4)

#define avdl_setUniform1ui(uniform, ui1)                glUniform1ui(uniform, ui1)
#define avdl_setUniform2ui(uniform, ui1, ui2)           glUniform2ui(uniform, ui1, ui2)
#define avdl_setUniform3ui(uniform, ui1, ui2, ui3)      glUniform3ui(uniform, ui1, ui2, ui3)
#define avdl_setUniform4ui(uniform, ui1, ui2, ui3, ui4) glUniform4ui(uniform, ui1, ui2, ui3, ui4)

/*
 * takes a string for the vertex and one for the fragment shader as parameters,
 * returns the number of the created program, or `0` if an error occured.
 */
unsigned int avdl_loadProgram(const char *vfname, const char *ffname);

/*
 * the default shader, draws vertex colors and textures only
 */
extern const char *avdl_shaderDefault_vertex;
extern const char *avdl_shaderDefault_fragment;

/*
 * the font shader, used only for text
 */
extern const char *avdl_shaderFont_vertex;
extern const char *avdl_shaderFont_fragment;

#endif
