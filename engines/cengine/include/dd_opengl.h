#ifndef DD_OPENGL_H
#define DD_OPENGL_H

// opengl id, used in case the context is lost and re-gained
void avdl_opengl_generateContextId();
int  avdl_opengl_getContextId();

// import opengl on android
#if DD_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// import opengl on native
#elif DD_PLATFORM_NATIVE
#include <GL/glew.h>
#include <SDL2/SDL.h>

#endif

#endif // DD_OPENGL_H
