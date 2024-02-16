#ifndef AVDL_SHADERS_H
#define AVDL_SHADERS_H

#include <avdl_graphics.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#ifdef AVDL_DIRECT3D11
int avdl_getUniformLocation(struct avdl_program *o, char *varname);
#else
GLint avdl_getUniformLocation(struct avdl_program *o, char *varname);
#endif

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
extern const char *avdl_shaderDefault_vertex_q2;
extern const char *avdl_shaderDefault_fragment_q2;

/*
 * the font shader, used only for text
 */
extern const char *avdl_shaderFont_vertex;
extern const char *avdl_shaderFont_fragment;
extern const char *avdl_shaderFont_vertex_q2;
extern const char *avdl_shaderFont_fragment_q2;

/*
 * skybox shaders
 */
extern const char *avdl_shaderSkybox_vertex;
extern const char *avdl_shaderSkybox_fragment;
extern const char *avdl_shaderSkybox_vertex_q2;
extern const char *avdl_shaderSkybox_fragment_q2;

#ifdef __cplusplus
}
#endif

#endif
