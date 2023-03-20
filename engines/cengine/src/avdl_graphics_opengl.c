#include "avdl_graphics.h"

#if DD_PLATFORM_NATIVE
#include "dd_opengl.h"
#endif

#include "avdl_shaders.h"
#include "dd_game.h"

void avdl_graphics_ClearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void avdl_graphics_ClearToColour() {
	glClearColor(dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b, 1);
}

void avdl_graphics_ClearColourAndDepth() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// context id - used to re-load assets if context is reset
static int openglContextId = 0;

void avdl_graphics_generateContextId() {
	openglContextId++;
}

int  avdl_graphics_getContextId() {
	return openglContextId;
}

// shaders
GLuint defaultProgram;
GLuint fontProgram;
GLuint currentProgram;

int avdl_graphics_Init() {

	avdl_graphics_generateContextId();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.8, 0.6, 1.0, 1);

	/*
	 * load shaders
	 */
	defaultProgram = avdl_loadProgram(avdl_shaderDefault_vertex, avdl_shaderDefault_fragment);
	if (!defaultProgram) {
		dd_log("avdl: error loading shaders");
		return -1;
	}

	fontProgram = avdl_loadProgram(avdl_shaderFont_vertex, avdl_shaderFont_fragment);
	if (!fontProgram) {
		dd_log("avdl: error loading font shaders");
		return -1;
	}

	glUseProgram(defaultProgram);
	currentProgram = defaultProgram;

	return 0;

}

void avdl_graphics_Viewport(int x, int y, int w, int h) {
	glViewport(x, y, w, h);
}

void avdl_graphics_PrintInfo() {
	dd_log("Vendor graphic card: %s", glGetString(GL_VENDOR));
	dd_log("Renderer: %s", glGetString(GL_RENDERER));
	dd_log("Version GL: %s", glGetString(GL_VERSION));
	dd_log("Version GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

int avdl_graphics_GetCurrentProgram() {
	int program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	return program;
}

void avdl_graphics_UseProgram(int program) {
	glUseProgram(program);
}

int avdl_graphics_GetUniformLocation(int program, const char *uniform) {
	return glGetUniformLocation(program, uniform);
}

void avdl_graphics_ImageToGpu(struct dd_image *o) {

	glGenTextures(1, &o->tex);
	glBindTexture(GL_TEXTURE_2D, o->tex);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#if DD_PLATFORM_NATIVE
	glTexImage2D(GL_TEXTURE_2D, 0, o->pixelFormat, o->width, o->height, 0, o->pixelFormat, GL_FLOAT, o->pixels);
	#elif DD_PLATFORM_ANDROID
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, o->width, o->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, o->pixelsb);
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	#if DD_PLATFORM_NATIVE
	free(o->pixels);
	o->pixels = 0;
	#elif DD_PLATFORM_ANDROID
	free(o->pixelsb);
	o->pixelsb = 0;
	#endif

}

void avdl_graphics_DeleteTexture(unsigned int tex) {
	glDeleteTextures(1, &tex);
}

void avdl_graphics_BindTexture(unsigned int tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
}

void avdl_graphics_EnableBlend() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void avdl_graphics_DisableBlend() {
	glDisable(GL_BLEND);
}

void avdl_graphics_EnableVertexAttribArray(int attrib) {
	glEnableVertexAttribArray(attrib);
}

void avdl_graphics_DisableVertexAttribArray(int attrib) {
	glDisableVertexAttribArray(attrib);
}

void avdl_graphics_VertexAttribPointer(int p, int size, int format, int normalised, int stride, void *data) {
	int norm;
	if (!normalised) {
		norm = GL_FALSE;
	}
	else {
		norm = GL_TRUE;
	}
	glVertexAttribPointer(p, size, format, norm, stride, data);
}

void avdl_graphics_DrawArrays(int vcount) {
	glDrawArrays(GL_TRIANGLES, 0, vcount);
}
