#ifndef AVDL_SHADERS_H
#define AVDL_SHADERS_H

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
