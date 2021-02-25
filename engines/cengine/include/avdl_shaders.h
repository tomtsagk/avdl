#ifndef DD_SHADERS_D
#define DD_SHADERS_D

unsigned int create_shader(int type, const char *src);
unsigned int create_program(unsigned int vsdr, unsigned int fsdr);

//Converts file contents to char* and calls create_shader with it
unsigned int load_shader(int type, const char *fname);

/* Takes filename vertex and fragment shader as parameters, 
 * calls load_shader for each of them,
 * returns create_program with the created shaders.
 */
unsigned int load_program(const char *vfname, const char *ffname);

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

#endif
