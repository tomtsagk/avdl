#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11

#include "avdl_shaders.h"
#include "dd_game.h"
#include "dd_log.h"
#include <stdlib.h>

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

int avdl_graphics_getContextId() {
	return openglContextId;
}

// shaders
GLuint defaultProgram;
GLuint currentProgram;

int avdl_graphics_Init() {

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	// init glew
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		dd_log("avdl: glew failed to initialise: %s\n", glewGetErrorString(glewError));
		return -1;
	}
	#endif

	avdl_graphics_generateContext();

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
	int program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	return program;
}

void avdl_graphics_UseProgram(int program) {
	glUseProgram(program);
}

int avdl_graphics_GetUniformLocation(int program, const char *uniform) {
	return glGetUniformLocation(program, uniform);
}

int avdl_graphics_ImageToGpu(void *pixels, int pixel_format, int width, int height) {

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
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
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_FLOAT, pixels);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pixels);
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;

}

void avdl_graphics_ImageToGpuUpdate(int texture_id, void *pixels, int pixel_format, int x, int y, int width, int height) {

	glBindTexture(GL_TEXTURE_2D, texture_id);

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	glTexSubImage2D(GL_TEXTURE_2D, 0,
		x,
		y,
		width,
		height,
		pixel_format,
		GL_FLOAT,
		pixels
	);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	glTexSubImage2D(GL_TEXTURE_2D, 0,
		x,
		y,
		width,
		height,
		pixel_format,
		GL_UNSIGNED_BYTE,
		pixels
	);
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

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

int avdl_graphics_generateContext() {

	avdl_graphics_generateContextId();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.8, 0.6, 1.0, 1);

	/*
	 * load shaders
	 */
	#if defined(AVDL_QUEST2)
	defaultProgram = avdl_loadProgram(avdl_shaderDefault_vertex_q2, avdl_shaderDefault_fragment_q2);
	#else
	defaultProgram = avdl_loadProgram(avdl_shaderDefault_vertex, avdl_shaderDefault_fragment);
	#endif
	if (!defaultProgram) {
		dd_log("avdl: error loading shaders");
		return -1;
	}

	glUseProgram(defaultProgram);
	currentProgram = defaultProgram;
	return 0;
}
#endif
