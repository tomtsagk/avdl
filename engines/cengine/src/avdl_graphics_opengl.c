#include "avdl_graphics.h"

#ifndef AVDL_DIRECT3D11

#include "avdl_shaders.h"
#include "dd_game.h"
#include "dd_log.h"
#include <stdlib.h>
#include "avdl_assetManager.h"
#include "dd_image.h"
#include "avdl_engine.h"
#include "dd_math.h"

void test_glError(char *file, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		switch (err) {
		case GL_INVALID_ENUM:
			dd_log("avdl: opengl error: invalid enum at %s %d", file, line);
			break;
		case GL_INVALID_VALUE:
			dd_log("avdl: opengl error: invalid value at %s %d", file, line);
			break;
		case GL_INVALID_OPERATION:
			dd_log("avdl: opengl error: invalid operation at %s %d", file, line);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			dd_log("avdl: opengl error: invalid framebuffer operation at %s %d", file, line);
			break;
		case GL_OUT_OF_MEMORY:
			dd_log("avdl: opengl error: out of memory at %s %d", file, line);
			break;
		// are these not present in OpenGL ES (Android)?
		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		case GL_STACK_UNDERFLOW:
			dd_log("stack underflow");
			break;
		case GL_STACK_OVERFLOW:
			dd_log("stack overflow");
			break;
		#endif
		}
	}
}

extern struct avdl_engine engine;

static void opengl_antialias(struct avdl_graphics *o) {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	if (o->antialias) {
		GL(glEnable(GL_MULTISAMPLE));
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, o->antialias_samples);
	}
	else {
		GL(glDisable(GL_MULTISAMPLE));
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}
	#endif
}

int avdl_graphics_CreateWindow(struct avdl_graphics *o) {

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// anti-aliasing
	opengl_antialias(o);

	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
	int width = dd_gameInitWindowWidth;
	int height = dd_gameInitWindowHeight;
	o->sdl_window = SDL_CreateWindow(gameTitle, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (o->sdl_window == NULL) {
		dd_log("avdl: failed to create SDL2 window: %s\n", SDL_GetError());
		return -1;
	}

	/*
	 * For RenderDoc ask OpenGl 3.2+ Core
	 */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	o->gl_context = SDL_GL_CreateContext(o->sdl_window);
	if (o->gl_context == NULL) {
		dd_log("avdl: failed to create OpenGL context: %s\n", SDL_GetError());
	}

	// init glew
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		dd_log("avdl: glew failed to initialise: %s\n", glewGetErrorString(glewError));
		return -1;
	}

	// window icon
	char filename[400];
	#if defined(_WIN32) || defined(WIN32)
	strcpy(filename, "assets/icon_64x64.png");
	#else
	strcpy(filename, avdl_getProjectLocation());
	strcat(filename, GAME_ASSET_PREFIX);
	strcat(filename, "assets/icon_64x64.png");
	#endif

	struct dd_image img;
	dd_image_create(&img);
	dd_image_load_png(&img, filename);
	if (img.pixels && img.pixelFormat == GL_RGBA) {
		SDL_Surface *surface;
		int size = sizeof(GLubyte) *img.width *img.height *4;
		GLubyte *pixels = malloc(size);
		for (int h = 0; h < img.height; h++) {
		for (int w = 0; w < img.width ; w++) {
			int iterator = (img.width *h *4) +(w *4);
			int iterator2 = (img.width *(img.height -1 -h) *4) +(w *4);
			pixels[iterator +0] = img.pixels[iterator2 +0] *255.0;
			pixels[iterator +1] = img.pixels[iterator2 +1] *255.0;
			pixels[iterator +2] = img.pixels[iterator2 +2] *255.0;
			pixels[iterator +3] = img.pixels[iterator2 +3] *255.0;
		}
		}
		surface = SDL_CreateRGBSurfaceFrom(
			pixels, // pixels
			img.width, img.height, // width height
			32, // depth
			img.width *(sizeof(GLubyte) * 4), // pitch
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 // color masks
		);
		SDL_SetWindowIcon(o->sdl_window, surface);
		SDL_FreeSurface(surface);
		free(pixels);
		dd_image_clean(&img);
	}
	#endif

	avdl_graphics_PrintInfo();
	avdl_graphics_generateContext();

	return 0;
}

int avdl_graphics_DestroyWindow(struct avdl_graphics *o) {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	SDL_GL_DeleteContext(o->gl_context);
	SDL_DestroyWindow(o->sdl_window);
	#endif
	return 0;
}

int avdl_graphics_SwapFramebuffer(struct avdl_graphics *o) {

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	SDL_GL_SwapWindow(o->sdl_window);
	#endif

	return 0;
}

void avdl_graphics_FullscreenToggle() {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	int isFullscreen = SDL_GetWindowFlags(engine.graphics.sdl_window) & FullscreenFlag;
	SDL_SetWindowFullscreen(engine.graphics.sdl_window, isFullscreen ? 0 : FullscreenFlag);
	#endif
}

void avdl_graphics_Refresh() {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	opengl_antialias(&engine.graphics);

	// destroy opengl context
	SDL_GL_DeleteContext(engine.graphics.gl_context);

	// recreate opengl context
	engine.graphics.gl_context = SDL_GL_CreateContext(engine.graphics.sdl_window);
	if (engine.graphics.gl_context == NULL) {
		dd_log("avdl: failed to create OpenGL context: %s\n", SDL_GetError());
	}

	// re-init glew
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		dd_log("avdl: glew failed to initialise: %s\n", glewGetErrorString(glewError));
		return -1;
	}

	// re-generate context
	avdl_graphics_generateContext();

	// re-create current world?
	//

	#endif
}

int avdl_graphics_CanFullscreenToggle() {
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	return 0;
#endif
	return 1;
}

void avdl_graphics_ClearDepth() {
	GL(glClear(GL_DEPTH_BUFFER_BIT));
}

void avdl_graphics_ClearToColour() {
	GL(glClearColor(dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b, 1));
}

void avdl_graphics_ClearColourAndDepth() {
	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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
	// Initialise SDL window
	int sdlError = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (sdlError < 0) {
		dd_log("avdl: error initialising SDL2: %s", SDL_GetError());
		return -1;
	}
	#endif

	return 0;

}

void avdl_graphics_Viewport(int x, int y, int w, int h) {
	GL(glViewport(x, y, w, h));
}

void avdl_graphics_PrintInfo() {
	dd_log("Vendor graphic card: %s", glGetString(GL_VENDOR));
	dd_log("Renderer: %s", glGetString(GL_RENDERER));
	dd_log("Version GL: %s", glGetString(GL_VERSION));
	dd_log("Version GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

int avdl_graphics_GetCurrentProgram() {
	int program = 0;
	GL(glGetIntegerv(GL_CURRENT_PROGRAM, &program));
	return program;
}

void avdl_graphics_UseProgram(int program) {
	GL(glUseProgram(program));
}

int avdl_graphics_GetUniformLocation(int program, const char *uniform) {
	return glGetUniformLocation(program, uniform);
}

avdl_texture_id avdl_graphics_ImageToGpu(void *pixels, int pixel_format, int width, int height) {

	GLuint tex;
	GL(glGenTextures(1, &tex));
	GL(glBindTexture(GL_TEXTURE_2D, tex));
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	*/

	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	GL(glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_FLOAT, pixels));
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL(glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pixels));
	#endif

	GL(glBindTexture(GL_TEXTURE_2D, 0));

	return tex;

}

void avdl_graphics_ImageToGpuUpdate(avdl_texture_id texture_id, void *pixels, int pixel_format, int x, int y, int width, int height) {

	GL(glBindTexture(GL_TEXTURE_2D, texture_id));

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	GL(glTexSubImage2D(GL_TEXTURE_2D, 0,
		x,
		y,
		width,
		height,
		pixel_format,
		GL_FLOAT,
		pixels
	));
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	GL(glTexSubImage2D(GL_TEXTURE_2D, 0,
		x,
		y,
		width,
		height,
		pixel_format,
		GL_UNSIGNED_BYTE,
		pixels
	));
	#endif

	GL(glBindTexture(GL_TEXTURE_2D, 0));

}

void avdl_graphics_DeleteTexture(avdl_texture_id tex) {
	GL(glDeleteTextures(1, &tex));
}

void avdl_graphics_BindTexture(avdl_texture_id tex) {
	GL(glBindTexture(GL_TEXTURE_2D, tex));
}

void avdl_graphics_EnableBlend() {
	GL(glEnable(GL_BLEND));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void avdl_graphics_DisableBlend() {
	GL(glDisable(GL_BLEND));
}

void avdl_graphics_EnableVertexAttribArray(int attrib) {
	GL(glEnableVertexAttribArray(attrib));
}

void avdl_graphics_DisableVertexAttribArray(int attrib) {
	GL(glDisableVertexAttribArray(attrib));
}

void avdl_graphics_VertexAttribPointer(int p, int size, int format, int normalised, int stride, void *data) {
	int norm;
	if (!normalised) {
		norm = GL_FALSE;
	}
	else {
		norm = GL_TRUE;
	}
	GL(glVertexAttribPointer(p, size, format, norm, stride, data));
}

void avdl_graphics_DrawArrays(int vcount) {
	GL(glDrawArrays(GL_TRIANGLES, 0, vcount));
}

int avdl_graphics_generateContext() {

	avdl_graphics_generateContextId();

	GL(glEnable(GL_DEPTH_TEST));
	GL(glClearColor(0.8, 0.6, 1.0, 1));

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

	GL(glUseProgram(defaultProgram));
	currentProgram = defaultProgram;
	return 0;
}

int avdl_graphics_setVSync(int flag) {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	// try to turn vsync on
	if (flag) {
		if (SDL_GL_SetSwapInterval(1) == 0) {
			// success
			return 0;
		}
		else {
			// vsync not supported
			return -1;
		}
	}
	else {
		if (SDL_GL_SetSwapInterval(0) == 0) {
			// success
			return 0;
		}
		else {
			// vsync not supported
			return -1;
		}
	}
	#endif
}

void avdl_graphics_SetMSAntiAlias(int samples) {
	samples = dd_math_max(samples, 1);
	samples = dd_math_min(samples, 8);
	engine.graphics.antialias = 1;
	engine.graphics.antialias_samples = samples;
}

void avdl_graphics_SetNoAntiAlias() {
	engine.graphics.antialias = 0;
}
#endif
