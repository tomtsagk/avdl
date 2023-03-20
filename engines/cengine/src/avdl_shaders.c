#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "avdl_cengine.h"

/*
 * intro shaders
 * these provide some macros depending
 * on the GLSL version, to device a more abstract
 * interface that works on multiple versions.
 *
 * the macros define a new language called
 * "avsl" - abstract video-game shading language
 */
static char *avsl_shader_vertex =
"#if __VERSION__ > 120\n"
"#define AVDL_IN in\n"
"#define AVDL_OUT out\n"
"#else\n"
"#define AVDL_IN attribute\n"
"#define AVDL_OUT varying\n"
"#endif\n"
;

static char *avsl_shader_fragment =
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
;

/*
 * supported GLSL versions
 * these are versions that "avsl" supports and
 * has been tested with.
 *
 * currently, the engine will try to compile
 * a shader with each version, until one succeeds
 * (or they all fail).
 */
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

/*
 * all shaders start with this version string,
 * where the version is assigned based on the values above
 * and compiled.
 */
const char *versionSource = "#version XXX YY\n";

unsigned int create_shader(int type, const char *src, int glslVersionIndex) {

	/*
	 * set up a copy of the source code, and try different versions of it
	 */
	if (!src || strlen(src) < 20) {
		dd_log("avdl: create_shader: no source given, or source smaller than 20");
		return 0;
	}

	/*
	 * find the right shader intro based on the shader
	 */
	char *shaderIntro;
	if (type == GL_VERTEX_SHADER) {
		shaderIntro = avsl_shader_vertex;
	}
	else
	if (type == GL_FRAGMENT_SHADER) {
		shaderIntro = avsl_shader_fragment;
	}
	else {
		dd_log("avdl: create_shader: unsupported shader type: %d", type);
		exit(-1);
	}

	/*
	 * construct a glsl shader based on the version, shader intro, and the shader itself
	 */
	char *newSource = malloc(strlen(shaderIntro) +strlen(src) +strlen(versionSource) +1);
	const char *newSource2 = newSource;
	strcpy(newSource, versionSource);
	strcat(newSource, shaderIntro);
	strcat(newSource, src);

	// potentially compile the actual glsl version first
	//dd_log("glsl version: %s", glGetStringi(GL_SHADING_LANGUAGE_VERSION, 0));

	// try with the given glsl version
	int i = glslVersionIndex;
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
	GLenum err = glGetError();
	if (!sdr || err != GL_NO_ERROR) {
		dd_log("avdl: create_shader: error creating shader");
		glDeleteShader(sdr);
		free(newSource);
		return 0;
	}
	glShaderSource(sdr, 1, &newSource2, 0);
	if (glGetError() != GL_NO_ERROR) {
		dd_log("avdl: create_shader: error getting shader source");
		glDeleteShader(sdr);
		free(newSource);
		return 0;
	}
	glCompileShader(sdr);
	GLint sdrStatus;
	glGetShaderiv(sdr, GL_COMPILE_STATUS, &sdrStatus);
	if (glGetError() != GL_NO_ERROR || sdrStatus == GL_FALSE) {
		dd_log("avdl: create_shader: error compiling shader source");

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

		glDeleteShader(sdr);
		free(newSource);
		return 0;
	}

	//Check compilation status
	int status;
	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);

	//Compilation failed
	if (!status)
	{
		//Delete shader
		glDeleteShader(sdr);
	}
	else {
		free(newSource);
		return sdr;
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

extern GLuint defaultProgram;
extern GLuint currentProgram;
void avdl_useProgram(struct avdl_program *o) {
	if (!o || !o->program) {
		glUseProgram(defaultProgram);
		currentProgram = defaultProgram;
	}
	else {
		glUseProgram(o->program);
		currentProgram = o->program;
	}
}

/*
 * load each shader, and link them into a program
 */
unsigned int avdl_loadProgram(const char *vfname, const char *ffname) {

	// check input
	if (!vfname || !ffname) return 0;

	// attempt to create shaders in all versions, first one to succeeds is accepted
	unsigned int vsdr = 0;
	unsigned int fsdr = 0;
	for (int i = 0; i < shader_glsl_versions_count; i++) {

		// create vertex shader
		if ( !(vsdr = create_shader(GL_VERTEX_SHADER, vfname, i)) ) {
			continue;
		}

		// create fragment shader
		if ( !(fsdr = create_shader(GL_FRAGMENT_SHADER, ffname, i)) )
		{
			glDeleteShader(vsdr);
			vsdr = 0;
			continue;
		}
		else {
			break;
		}
	}

	if (!vsdr || !fsdr) {
		dd_log("avdl: one or more shaders failed to compile: "
			"vertex('%s') = %d - fragment('%s') = %d", vfname, vsdr, ffname, fsdr);
		return 0;
	}

	// shaders created - create program
	return create_program(vsdr, fsdr);
}

const char *avdl_shaderDefault_vertex =
"AVDL_IN vec4 position;\n"
"AVDL_IN vec3 colour;\n"
"AVDL_IN vec2 texCoord;\n"

"uniform mat4 matrix;\n"

"AVDL_OUT vec2 outTexCoord;\n"
"AVDL_OUT vec4 outColour;\n"

"void main() {\n"
"	gl_Position = matrix *position;\n"
"	outColour = vec4(colour.rgb, 0);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;

const char *avdl_shaderDefault_fragment =
"AVDL_IN vec4 outColour;\n"
"AVDL_IN vec2 outTexCoord;\n"

"uniform sampler2D image;\n"

"void main() {\n"
"	avdl_frag_color = outColour +avdl_texture(image, outTexCoord);\n"
"}\n"
;

const char *avdl_shaderFont_vertex =
"AVDL_IN vec4 position;\n"
"AVDL_IN vec2 texCoord;\n"

"uniform vec3 colorFront;\n"
"uniform vec3 colorBack;\n"
"uniform mat4 matrix;\n"

"AVDL_OUT vec4 outColour;\n"
"AVDL_OUT vec2 outTexCoord;\n"

"void main() {\n"
"	gl_Position = matrix *position;\n"
"	outColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"	outTexCoord  = texCoord;\n"
"}\n"
;

const char *avdl_shaderFont_fragment =
"AVDL_IN vec4 outColour;\n"
"AVDL_IN vec2 outTexCoord;\n"

"uniform sampler2D image;\n"

"void main() {\n"
"	vec4 finalCol = avdl_texture(image, outTexCoord);\n"
"	if (finalCol.r < 0.05 && finalCol.g < 0.05 && finalCol.b < 0.05) {\n"
"		discard;\n"
"	}\n"
"	avdl_frag_color = finalCol;\n"
"}\n"
;

void avdl_program_create(struct avdl_program *o) {
	o->sdrVertexSrc = 0;
	o->sdrFragmentSrc = 0;
	o->openglContext = 0;
	o->program = 0;

	o->clean = avdl_program_clean;
	o->setVertexShader = avdl_program_setVertexShader;
	o->setFragmentShader = avdl_program_setFragmentShader;
	o->useProgram = avdl_program_useProgram;
}

void avdl_program_clean(struct avdl_program *o) {
}

void avdl_program_setVertexShader(struct avdl_program *o, char *source) {
	o->sdrVertexSrc = source;
}

void avdl_program_setFragmentShader(struct avdl_program *o, char *source) {
	o->sdrFragmentSrc = source;
}

void avdl_program_useProgram(struct avdl_program *o) {
	if (o->program == 0
	||  o->openglContext != avdl_graphics_getContextId()) {
		o->openglContext = avdl_graphics_getContextId();
		o->program = avdl_loadProgram(o->sdrVertexSrc, o->sdrFragmentSrc);
	}
	avdl_useProgram(o);
}

GLint avdl_getUniformLocation(struct avdl_program *o, char *varname) {
	return glGetUniformLocation(o->program, varname);
}
