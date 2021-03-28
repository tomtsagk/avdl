#include "dd_opengl.h"

static int openglContextId = 0;

void avdl_opengl_generateContextId() {
	openglContextId++;
}

int  avdl_opengl_getContextId() {
	return openglContextId;
}
