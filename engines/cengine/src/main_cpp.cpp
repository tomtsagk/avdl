#include "avdl_engine.h"

extern "C" int dd_main(int argc, char *argv[]);

extern "C" struct avdl_engine engine;

#ifdef AVDL_STEAM
#ifndef AVDL_DIRECT3D11
#ifdef __cplusplus
extern "C"
#endif
#if defined(_WIN32) || defined(WIN32)
int SDL_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
	return dd_main(argc, argv);
}
#endif
#endif

#if 0
#ifdef AVDL_DIRECT3D11
#include "avdl_engine.h"
#include <stdio.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	engine.hInstance = hInstance;
	engine.nCmdShow = nCmdShow;
	//avdl_engine_init(&engine);

	//printf("Hello world");
	return dd_main(0, 0);
	//return 0;
}
#endif
#endif
