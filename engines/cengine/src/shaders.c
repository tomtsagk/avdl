#include <stdio.h>
#include <stdlib.h>
#include "shaders.h"
#include <errno.h>
#include <string.h>
#include <dd_log.h>
#include "dd_opengl.h"

unsigned int create_shader(int type, const char *src)
{
	//Create shader and compile it
	unsigned int sdr = glCreateShader(type);
	glShaderSource(sdr, 1, &src, 0);
	glCompileShader(sdr);

	//Check compilation status
	int status;
	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);

	//Compilation failed
	if (!status)
	{
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
		
		//Delete shader and return nothing
		glDeleteShader(sdr);
		return 0;
	}

	//Return shader
	return sdr;
}

unsigned int create_program(unsigned int vsdr, unsigned int fsdr)
{
	//No shaders given
	if (!vsdr && !fsdr) {
		dd_log("create_program called with invalid shader objects, aborting");
		return 0;
	}

	//Create program, attach shaders and link program
	unsigned int prog = glCreateProgram();
	if (vsdr) glAttachShader(prog, vsdr);
	if (fsdr) glAttachShader(prog, fsdr);
	glBindAttribLocation(prog, 0, "position");
	glBindAttribLocation(prog, 1, "colour");
	glBindAttribLocation(prog, 2, "texCoord");
	glLinkProgram(prog);

	//Check compilation status
	int status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);

	//Compilation failed
	if (!status) {
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

		//Delete program and return nothing
		glDeleteProgram(prog);
		return 0;
	}

	//Return program
	return prog;
}

unsigned int load_shader(int type, const char *shader)
{
	//Create shader
	unsigned int sdr = create_shader(type, shader);
	/*
	if (type == GL_VERTEX_SHADER) {
	}
	else {
		//Open shader's file
		FILE *fp = fopen(fname, "rb");
		if (!fp) {
			dd_log("failed to open shader \"%s\": %s", fname, strerror(errno));
			return 0;
		}

		//Calculate amount of characters
		fseek(fp, 0, SEEK_END);
		int sz = ftell(fp);
		rewind(fp);

		//Create new char array to hold file's content (terminating with 0)
		char *src = malloc(sizeof(char) *(sz + 1));
		fread(src, 1, sz, fp);
		src[sz] = 0;
		fclose(fp);
		sdr = create_shader(type, src);
		free(src);
	}
	*/

	//Return shader's number
	return sdr;
}

unsigned int load_program(const char *vfname, const char *ffname)
{
	//Vertex shader
	unsigned int vsdr = 0;
	if (vfname) {
		if ( !(vsdr = load_shader(GL_VERTEX_SHADER, vfname)) ) {
			return 0;
		}
	}

	//Fragment shader
	unsigned int fsdr = 0;
	if (ffname) {
		if ( !(fsdr = load_shader(GL_FRAGMENT_SHADER, ffname)) )
		{
			glDeleteShader(vsdr);
			return 0;
		}
	}

	//Shaders created - create program
	return create_program(vsdr, fsdr);
}

const char *shader_vertex =
"#version 130\n"
"in vec4 position;\n"
"in vec3 colour;\n"
"in vec2 texCoord;\n"
"uniform mat4 matrix;\n"
"uniform mat4 matrixProjection;\n"
"out vec2 outTexCoord;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"
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
"uniform mat4 matrixProjection;\n"
"out vec2 outTexCoord;\n"
"out vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"
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
"uniform mat4 matrixProjection;\n"
"varying vec2 outTexCoord;\n"
"varying vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"
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
"uniform mat4 matrixProjection;\n"
"out vec2 outTexCoord;\n"
"uniform float animationMax;\n"
"uniform float animationCurrent;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"

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
"uniform mat4 matrixProjection;\n"
"varying vec2 outTexCoord;\n"
"uniform float animationMax;\n"
"uniform float animationCurrent;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"

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
"uniform mat4 matrixProjection;\n"
"out vec2 outTexCoord;\n"
"out vec4 outColour;\n"
"void main() {\n"
"	vec4 pos = matrixProjection *matrix *position;\n"

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
"uniform mat4 matrixProjection;\n"
"varying vec2 outTexCoord;\n"
"varying vec4 outColour;\n"
"void main() {\n"
"	vec4 pos = matrixProjection *matrix *position;\n"

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
