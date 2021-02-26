#ifndef AVDL_SHADERS_D
#define AVDL_SHADERS_D

/*
 * Takes a string for the vertex and one for the fragment shader as parameters,
 * returns the number of the created program, or `0` if an error occured.
 */
unsigned int load_program(const char *vfname, const char *ffname);

extern const char *shader_vertex_universal;
extern const char *shader_fragment_universal;
/*
extern const char *shader_vertex;
extern const char *shader_vertex110;
extern const char *shader_vertexES;
extern const char *shader_vertexES101;
extern const char *shader_rising_vertex;
extern const char *shader_rising_vertex110;
extern const char *shader_rising_vertexES;
extern const char *shader_rising_vertexES101;
extern const char *shader_fragment;
extern const char *shader_fragment110;
extern const char *shader_fragmentES;
extern const char *shader_fragmentES101;
*/

#endif
