#ifndef AVDL_GRAPHICS_H
#define AVDL_GRAPHICS_H

// import opengl on android
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <EGL/egl.h>
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

// direct3d 11
#elif AVDL_DIRECT3D11
/*
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#ifdef AVDL_SCARLETT
	#include <dxgi1_2.h>
#else
	#include <dxgi.h>
#endif
#include <d3dcompiler.h>
#include <assert.h>

#include <Windows.h>

#ifndef UNICODE
#define UNICODE
#endif
*/

// import opengl on native
#elif defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
#include <GL/glew.h>

#if defined(_WIN32) || defined(WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

void avdl_graphics_ClearDepth();
void avdl_graphics_ClearToColour();
void avdl_graphics_ClearColourAndDepth();

// abstract mesh
typedef void avdl_graphics_mesh;

// opengl id, used in case the context is lost and re-gained
void avdl_graphics_generateContextId();
int  avdl_graphics_getContextId();

int avdl_graphics_generateContext();

#if defined( AVDL_DIRECT3D11 )
#define avdl_graphics_SetUniformMatrix4f(uniform, matrix)

#define avdl_graphics_SetUniform1f(uniform, f1)             
#define avdl_graphics_SetUniform2f(uniform, f1, f2)         
#define avdl_graphics_SetUniform3f(uniform, f1, f2, f3)     
#define avdl_graphics_SetUniform4f(uniform, f1, f2, f3, f4) 

#define avdl_graphics_SetUniform1i(uniform, i1)             
#define avdl_graphics_SetUniform2i(uniform, i1, i2)         
#define avdl_graphics_SetUniform3i(uniform, i1, i2, i3)     
#define avdl_graphics_SetUniform4i(uniform, i1, i2, i3, i4) 

#define avdl_graphics_SetUniform1ui(uniform, ui1)                
#define avdl_graphics_SetUniform2ui(uniform, ui1, ui2)           
#define avdl_graphics_SetUniform3ui(uniform, ui1, ui2, ui3)      
#define avdl_graphics_SetUniform4ui(uniform, ui1, ui2, ui3, ui4) 

#else

#define avdl_graphics_SetUniformMatrix4f(uniform, matrix) glUniformMatrix4fv(uniform, 1, GL_FALSE, matrix)

#define avdl_graphics_SetUniform1f(uniform, f1)             glUniform1f(uniform, f1)
#define avdl_graphics_SetUniform2f(uniform, f1, f2)         glUniform2f(uniform, f1, f2)
#define avdl_graphics_SetUniform3f(uniform, f1, f2, f3)     glUniform3f(uniform, f1, f2, f3)
#define avdl_graphics_SetUniform4f(uniform, f1, f2, f3, f4) glUniform4f(uniform, f1, f2, f3, f4)

#define avdl_graphics_SetUniform1i(uniform, i1)             glUniform1i(uniform, i1)
#define avdl_graphics_SetUniform2i(uniform, i1, i2)         glUniform2i(uniform, i1, i2)
#define avdl_graphics_SetUniform3i(uniform, i1, i2, i3)     glUniform3i(uniform, i1, i2, i3)
#define avdl_graphics_SetUniform4i(uniform, i1, i2, i3, i4) glUniform4i(uniform, i1, i2, i3, i4)

#define avdl_graphics_SetUniform1ui(uniform, ui1)                glUniform1ui(uniform, ui1)
#define avdl_graphics_SetUniform2ui(uniform, ui1, ui2)           glUniform2ui(uniform, ui1, ui2)
#define avdl_graphics_SetUniform3ui(uniform, ui1, ui2, ui3)      glUniform3ui(uniform, ui1, ui2, ui3)
#define avdl_graphics_SetUniform4ui(uniform, ui1, ui2, ui3, ui4) glUniform4ui(uniform, ui1, ui2, ui3, ui4)

#endif

int avdl_graphics_Init();

void avdl_graphics_Viewport(int x, int y, int w, int h);
void avdl_graphics_PrintInfo();

int avdl_graphics_GetCurrentProgram();
void avdl_graphics_UseProgram(int program);

int avdl_graphics_GetUniformLocation(int program, const char *uniform);

void avdl_graphics_EnableBlend();
void avdl_graphics_DisableBlend();

void avdl_graphics_EnableVertexAttribArray(int attrib);
void avdl_graphics_DisableVertexAttribArray(int attrib);
void avdl_graphics_VertexAttribPointer(int p, int size, int format, int, int, void *data);

void avdl_graphics_DrawArrays(int vcount);

// Textures
#if defined( AVDL_DIRECT3D11 )
//typedef ID3D11ShaderResourceView avdl_texture_id;
typedef void* avdl_texture_id;
#else
typedef int avdl_texture_id;
#endif

void avdl_graphics_DeleteTexture(avdl_texture_id tex);
void avdl_graphics_BindTexture(avdl_texture_id tex);

avdl_texture_id avdl_graphics_ImageToGpu(void *pixels, int pixel_format, int width, int height);
void avdl_graphics_ImageToGpuUpdate(avdl_texture_id texture_id, void *pixels, int pixel_format, int x, int y, int width, int height);

#if AVDL_DIRECT3D11
void avdl_graphics_d3d11_SetWindow();
#endif

#ifdef __cplusplus
}
#endif

#endif
