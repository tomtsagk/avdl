#ifndef DD_OPENGL_H
#define DD_OPENGL_H

// import opengl on android
#if DD_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// import opengl on native
#elif DD_PLATFORM_NATIVE
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#endif

#endif // DD_OPENGL_H
