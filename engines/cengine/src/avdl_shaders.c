#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "avdl_cengine.h"

unsigned int create_shader(int type, const char *src)
{
	/*
	 * create shader and compile it
	 */
	unsigned int sdr = glCreateShader(type);
	if (!sdr || glGetError() != GL_NO_ERROR) {
		dd_log("avdl: create_shader: error creating shader");
		return 0;
	}
	glShaderSource(sdr, 1, &src, 0);
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
		return 0;
	}

	//Return shader
	return sdr;
}

unsigned int create_program(unsigned int vsdr, unsigned int fsdr)
{
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
unsigned int load_program(const char *vfname, const char *ffname)
{
	// vertex shader
	unsigned int vsdr = 0;
	if (vfname) {
		if ( !(vsdr = create_shader(GL_VERTEX_SHADER, vfname)) ) {
			return 0;
		}
	}

	// fragment shader
	unsigned int fsdr = 0;
	if (ffname) {
		if ( !(fsdr = create_shader(GL_FRAGMENT_SHADER, ffname)) )
		{
			glDeleteShader(vsdr);
			return 0;
		}
	}

	// shaders created - create program
	return create_program(vsdr, fsdr);
}

const char *shader_vertex_universal =
"#version XXX YY\n"

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

const char *shader_vertex =
"#version 130\n"
"in vec4 position;\n"
"in vec3 colour;\n"
"in vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"out vec2 outTexCoord;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"
"	gl_FrontColor = vec4(colour.rgb, 1);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;
const char *shader_vertex110 =
"#version 110\n"
"attribute vec4 position;\n"
"attribute vec3 colour;\n"
"attribute vec2 texCoord;\n"
"varying vec2 outTexCoord;\n"
"void main() {\n"
"	gl_Position = gl_ProjectionMatrix *gl_ModelViewMatrix *position;\n"
"	gl_FrontColor = vec4(colour.rgb, 1);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;
const char *shader_vertexES =
"#version 310 es\n"
"in vec4 position;\n"
"in vec3 colour;\n"
"in vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"out vec2 outTexCoord;\n"
"out vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"
"	outTexCoord  = texCoord;\n"
"	outColour  = vec4(colour.rgb, 1);\n"
"}\n"
;
const char *shader_vertexES101 =
"#version 101\n"
"attribute vec4 position;\n"
"attribute vec3 colour;\n"
"attribute vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"varying vec2 outTexCoord;\n"
"varying vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"
"	outTexCoord  = texCoord;\n"
"	outColour  = vec4(colour.rgb, 1);\n"
"}\n"
;
const char *shader_fragment =
"#version 130\n"
"uniform sampler2D image;\n"
"in vec2 outTexCoord;\n"
"void main() {\n"
"	gl_FragColor = gl_Color;\n"
"	gl_FragColor += texture2D(image, outTexCoord);\n"
"}\n"
;
const char *shader_fragment110 =
"#version 110\n"
"uniform sampler2D image;\n"
"varying vec2 outTexCoord;\n"
"void main() {\n"
"	gl_FragColor = gl_Color;\n"
"	gl_FragColor += texture2D(image, outTexCoord);\n"
"}\n"
;
const char *shader_fragmentES =
"#version 310 es\n"
"uniform sampler2D image;\n"
"in highp vec2 outTexCoord;\n"
"in highp vec4 outColour;\n"
"layout(location = 0) out highp vec4 frag_color;\n"
"void main() {\n"
"	frag_color = outColour;\n"
"	frag_color += texture(image, outTexCoord);\n"
"}\n"
;
const char *shader_fragmentES101 =
//"#version 101\n"
"uniform sampler2D image;\n"
"varying mediump vec2 outTexCoord;\n"
"varying mediump vec4 outColour;\n"
"void main() {\n"
"	gl_FragColor = outColour;\n"
"	gl_FragColor += texture2D(image, outTexCoord);\n"
"}\n"
;

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

/*
const char *shader_fragment =
"#version 130\n"
"uniform sampler2D image;\n"
"in vec2 outTexCoord;\n"
"uniform float time;\n"
"float rand(vec2 co) {"
"	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);"
"}"
"void main() {\n"
"	gl_FragColor = gl_Color;\n"
"	gl_FragColor += texture2D(image, outTexCoord);\n"
"	gl_FragColor += 0.2 *rand(vec2(time, time)) -0.1;\n"
"}\n"
;
*/
