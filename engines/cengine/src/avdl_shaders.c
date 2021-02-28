#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "avdl_cengine.h"

const char *shader_glsl_versions[] = {
	// GLSL
	"110",
	"120",
	"130",
	"140",
	"150",
	"330",

	// GLSL ES
	"100",
	"101",
	"300 es",
	"310 es",
	"320 es",

	/* Future versions to potentially support
	"400",
	"410",
	"420",
	"430",
	"440",
	"450",
	*/
};
const int shader_glsl_versions_count = sizeof(shader_glsl_versions) /sizeof(char *);

const char *versionSource = "#version XXX YY\n";

unsigned int create_shader(int type, const char *src) {

	/*
	 * set up a copy of the source code, and try different versions of it
	 */
	if (!src || strlen(src) < 20) {
		dd_log("avdl: create_shader: no source given, or source smaller than 20");
		return 0;
	}
	char *newSource = malloc(strlen(src) +strlen(versionSource) +1);
	const char *newSource2 = newSource;
	strcpy(newSource, versionSource);
	strcat(newSource, src);

	// potentially compile the actual glsl version first
	//dd_log("glsl version: %s", glGetStringi(GL_SHADING_LANGUAGE_VERSION, 0));

	// try all supported versions, until one works
	for (int i = 0; i < shader_glsl_versions_count; i++) {
		newSource[ 9] = shader_glsl_versions[i][0];
		newSource[10] = shader_glsl_versions[i][1];
		newSource[11] = shader_glsl_versions[i][2];

		if (strlen(shader_glsl_versions[i]) >= 6) {
			newSource[13] = shader_glsl_versions[i][4];
			newSource[14] = shader_glsl_versions[i][5];
		}
		else {
			newSource[13] = ' ';
			newSource[14] = ' ';
		}

		/*
		 * create shader and compile it
		 */
		unsigned int sdr = glCreateShader(type);
		if (!sdr || glGetError() != GL_NO_ERROR) {
			dd_log("avdl: create_shader: error creating shader");
			return 0;
		}
		glShaderSource(sdr, 1, &newSource2, 0);
		if (glGetError() != GL_NO_ERROR) {
			dd_log("avdl: create_shader: error getting shader source");
			return 0;
		}
		glCompileShader(sdr);
		if (glGetError() != GL_NO_ERROR) {
			dd_log("avdl: create_shader: error compiling shader source");
			return 0;
		}

		//Get compilation log
		int logsz;
		glGetShaderiv(sdr, GL_INFO_LOG_LENGTH, &logsz);
		if (logsz > 1) {
			char *buf = malloc(sizeof(char) *(logsz +1));
			glGetShaderInfoLog(sdr, logsz, 0, buf);
			buf[logsz] = 0;
			dd_log("avdl: compilation of %s shader failed: %s",
				type == GL_VERTEX_SHADER   ? "vertex" :
				type == GL_FRAGMENT_SHADER ? "fragment" :
				"<unknown>", buf);
			free(buf);
		}

		//Check compilation status
		int status;
		glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);

		//Compilation failed
		if (!status)
		{
			//Delete shader and return nothing
			glDeleteShader(sdr);
			continue;
		}
		else {
			free(newSource);
			return sdr;
		}
	}
	free(newSource);

	// no shader compiled
	return 0;
}

unsigned int create_program(unsigned int vsdr, unsigned int fsdr) {
	// no shaders given
	if (!vsdr || !fsdr) {
		dd_log("create_program called with invalid shader objects, aborting");
		return 0;
	}

	// create program, attach shaders and link program
	unsigned int prog = glCreateProgram();
	glAttachShader(prog, vsdr);
	glAttachShader(prog, fsdr);
	glBindAttribLocation(prog, 0, "position");
	glBindAttribLocation(prog, 1, "colour");
	glBindAttribLocation(prog, 2, "texCoord");
	glLinkProgram(prog);

	//Get compilation log
	int logsz;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logsz);
	if (logsz > 1) {
		char *buf = malloc(sizeof(char) *(logsz +1));
		glGetProgramInfoLog(prog, logsz, 0, buf);
		buf[logsz] = 0;
		dd_log("linking shader program failed.\nshader program linker log: %s", buf);
		free(buf);
	}

	//Check compilation status
	int status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);

	//Compilation failed
	if (!status) {

		//Delete program and return nothing
		glDeleteProgram(prog);
		return 0;
	}

	//Return program
	return prog;
}

/*
 * load each shader, and link them into a program
 */
unsigned int avdl_loadProgram(const char *vfname, const char *ffname) {

	// check input
	if (!vfname || !ffname) return 0;

	// create vertex shader
	unsigned int vsdr = 0;
	if ( !(vsdr = create_shader(GL_VERTEX_SHADER, vfname)) ) {
		return 0;
	}

	// create fragment shader
	unsigned int fsdr = 0;
	if ( !(fsdr = create_shader(GL_FRAGMENT_SHADER, ffname)) )
	{
		glDeleteShader(vsdr);
		return 0;
	}

	// shaders created - create program
	return create_program(vsdr, fsdr);
}

const char *avdl_shaderDefault_vertex =
"#if __VERSION__ > 120\n"
"#define AVDL_IN in\n"
"#define AVDL_OUT out\n"
"#else\n"
"#define AVDL_IN attribute\n"
"#define AVDL_OUT varying\n"
"#endif\n"

"AVDL_IN vec4 position;\n"
"AVDL_IN vec3 colour;\n"
"AVDL_IN vec2 texCoord;\n"

"uniform mat4 matrix;\n"

"AVDL_OUT vec2 outTexCoord;\n"
"AVDL_OUT vec4 outColour;\n"

"void main() {\n"
"	gl_Position = matrix *position;\n"
"	outColour = vec4(colour.rgb, 1);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;

const char *avdl_shaderDefault_fragment =
"#if __VERSION__ > 120 || __VERSION__ == 100\n"
"precision mediump float;\n"
"#endif\n"

"#if __VERSION__ > 120\n"
"#define AVDL_IN in\n"
"#else\n"
"#define AVDL_IN varying\n"
"#endif\n"

"#if __VERSION__ > 150\n"
"layout(location = 0) out mediump vec4 frag_color;\n"
"#define avdl_frag_color frag_color\n"
"#define avdl_texture(x, y) texture(x, y)\n"
"#else\n"
"#define avdl_frag_color gl_FragColor\n"
"#define avdl_texture(x, y) texture2D(x, y)\n"
"#endif\n"

"AVDL_IN vec4 outColour;\n"
"AVDL_IN vec2 outTexCoord;\n"

"uniform sampler2D image;\n"

"void main() {\n"
"	avdl_frag_color = outColour +avdl_texture(image, outTexCoord);\n"
"}\n"
;

/*
 * the rising shader was created for a specific game, so
 * it will eventually be removed from here and moved there.
 *
const char *shader_rising_vertex =
"#version 130\n"
"in vec4 position;\n"
"in vec3 colour;\n"
"in vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"out vec2 outTexCoord;\n"
"uniform float animationMax;\n"
"uniform float animationCurrent;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"

"	if (-position.z > animationCurrent) {\n"
"		if (-position.z -animationCurrent > 5) {\n"
"			gl_Position.y -= 50;\n"
"		}\n"
"		else {\n"
"			float ratio = (-position.z -animationCurrent) /5;\n"
"			gl_Position.y -= ((ratio *ratio) -0.3) *ratio *1.43 *50;\n"
"		}\n"
"	}\n"

"	gl_FrontColor = vec4(colour.rgb, 1);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;
const char *shader_rising_vertex110 =
"#version 110\n"
"attribute vec4 position;\n"
"attribute vec3 colour;\n"
"attribute vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"varying vec2 outTexCoord;\n"
"uniform float animationMax;\n"
"uniform float animationCurrent;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"

"	if (-position.z > animationCurrent) {\n"
"		if (-position.z -animationCurrent > 5.0) {\n"
"			gl_Position.y -= 50.0;\n"
"		}\n"
"		else {\n"
"			float ratio = (-position.z -animationCurrent) /5.0;\n"
"			gl_Position.y -= ((ratio *ratio) -0.3) *ratio *1.43 *50.0;\n"
"		}\n"
"	}\n"

"	gl_FrontColor = vec4(colour.rgb, 1);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;
const char *shader_rising_vertexES =
"#version 310 es\n"
"in vec4 position;\n"
"in vec3 colour;\n"
"in vec2 texCoord;\n"
"uniform float animationCurrent;\n"
"uniform mat4 matrix;\n"
"out vec2 outTexCoord;\n"
"out vec4 outColour;\n"
"void main() {\n"
"	vec4 pos = matrix *position;\n"

"	if (-position.z > animationCurrent) {\n"
"		if (-position.z -animationCurrent > 5.0) {\n"
"			pos.y -= 50.0;\n"
"		}\n"
"		else {\n"
"			float ratio = (-position.z -animationCurrent) /5.0;\n"
"			pos.y -= ((ratio *ratio) -0.3) *ratio *1.43 *50.0;\n"
"		}\n"
"	}\n"
"	gl_Position = pos;\n"

"	outColour = vec4(colour.rgb, 1.0);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;

const char *shader_rising_vertexES101 =
"#version 101\n"
"attribute vec4 position;\n"
"attribute vec3 colour;\n"
"attribute vec2 texCoord;\n"
"uniform float animationCurrent;\n"
"uniform mat4 matrix;\n"
"varying vec2 outTexCoord;\n"
"varying vec4 outColour;\n"
"void main() {\n"
"	vec4 pos = matrix *position;\n"

"	if (-position.z > animationCurrent) {\n"
"		if (-position.z -animationCurrent > 5.0) {\n"
"			pos.y -= 50.0;\n"
"		}\n"
"		else {\n"
"			float ratio = (-position.z -animationCurrent) /5.0;\n"
"			pos.y -= ((ratio *ratio) -0.3) *ratio *1.43 *50.0;\n"
"		}\n"
"	}\n"

"	gl_Position = pos;\n"
"	outColour = vec4(colour.rgb, 1.0);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;
*/