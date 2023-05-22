#ifndef AVDL_ENGINE_H
#define AVDL_ENGINE_H

#include "avdl_graphics.h"
#include "dd_world.h"
#include "avdl_input.h"
#include "avdl_achievements.h"
#include "avdl_time.h"
#include "dd_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_EYES 2

#if defined(AVDL_QUEST2)
#include <android/native_window_jni.h>

struct Element {
	GLuint ColorTexture;
	GLuint DepthTexture;
	GLuint FrameBufferObject;
};

union ovrCompositorLayer_Union {
    XrCompositionLayerProjection Projection;
    XrCompositionLayerQuad Quad;
    XrCompositionLayerCylinderKHR Cylinder;
    XrCompositionLayerCubeKHR Cube;
    XrCompositionLayerEquirectKHR Equirect;
};

enum { ovrMaxLayerCount = 16 };
#endif

struct avdl_engine {

	#ifdef AVDL_DIRECT3D11
	/*
	HINSTANCE hInstance;
	int nCmdShow;
	HWND hwnd;
	*/
	#else
	#if DD_PLATFORM_NATIVE
	SDL_Window *window;
	SDL_GLContext glContext;
	#endif

	#endif

	int isRunning;
	int isPaused;

	struct dd_world *cworld;
	struct dd_world *nworld;
	int nworld_ready;
	int nworld_loading;
	void (*nworld_constructor)(struct dd_world*);
	int nworld_size;

	// settings
	int verify;
	int quiet;

	struct avdl_achievements *achievements;

	struct AvdlInput input;

	struct avdl_time end_of_update_time;

	#if defined(AVDL_QUEST2)

	// projection and view matrices for each eye
	GLuint SceneMatrices;
        struct dd_matrix View[NUM_EYES];
        struct dd_matrix Proj[NUM_EYES];

	// should controllers be rendered
        int RenderController[2];

	// controller matrices
	struct dd_matrix ControllerPoses[2];

	// default framebuffer
	int Multisamples;
	uint32_t SwapChainLength;
	struct Element* Elements;
	GLuint* colorTextures;
	int swapChainIndex;

	// OpenXR variables
	XrSwapchain ColorSwapChain;
	ANativeWindow* NativeWindow;

	int resumed;
	int focused;
	int frameCount;
	int SessionActive;

	XrSession session;

	XrSpace HeadSpace;
	XrSpace LocalSpace;
	XrSystemId SystemId;

	XrViewConfigurationProperties ViewportConfig;
	XrViewConfigurationView ViewConfigurationView[NUM_EYES];
	union ovrCompositorLayer_Union Layers[ovrMaxLayerCount];
	int LayerCount;
	#endif
};

int avdl_engine_init(struct avdl_engine *o);
int avdl_engine_initWorld(struct avdl_engine *o, void (*constructor)(struct dd_world*), int size);
int avdl_engine_clean(struct avdl_engine *o);
int avdl_engine_draw(struct avdl_engine *o);

int avdl_engine_resize(struct avdl_engine *o, int w, int h);
int avdl_engine_update(struct avdl_engine *o, float dt);
int avdl_engine_loop(struct avdl_engine *o);

void avdl_engine_setPaused(struct avdl_engine *o, int state);
int avdl_engine_isPaused(struct avdl_engine *o);

void avdl_engine_verify(struct avdl_engine *o);

#ifdef __cplusplus
}
#endif

#endif
