#include "avdl_engine.h"
#include "dd_game.h"

#include "dd_matrix.h"
#include "dd_fov.h"
#include "avdl_graphics.h"
#include "avdl_cengine.h"

#include <math.h>
#include <string.h>


#if defined(AVDL_QUEST2)
// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif

#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
#endif

#if !defined(GL_EXT_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height);
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)(
	GLenum target,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level,
	GLsizei samples);
#endif

#if !defined(GL_OVR_multiview)
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint baseViewIndex,
	GLsizei numViews);
#endif

#if !defined(GL_OVR_multiview_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLsizei samples,
	GLint baseViewIndex,
	GLsizei numViews);
#endif

struct OpenGLExtensions_t {
	int multi_view; // GL_OVR_multiview, GL_OVR_multiview2
	int EXT_texture_border_clamp; // GL_EXT_texture_border_clamp, GL_OES_texture_border_clamp
	char *EXT_sRGB_write_control;
};
struct OpenGLExtensions_t glExtensions;

static void EglInitExtensions() {
	//glExtensions = {};
	const char* allExtensions = (const char*)glGetString(GL_EXTENSIONS);
	if (allExtensions != 0) {
		glExtensions.multi_view = strstr(allExtensions, "GL_OVR_multiview2") &&
		strstr(allExtensions, "GL_OVR_multiview_multisampled_render_to_texture");

		glExtensions.EXT_texture_border_clamp =
			strstr(allExtensions, "GL_EXT_texture_border_clamp") ||
			strstr(allExtensions, "GL_OES_texture_border_clamp");
		glExtensions.EXT_sRGB_write_control = strstr(allExtensions, "GL_EXT_sRGB_write_control");
	}
	else {
		glExtensions.multi_view = 0;
		glExtensions.EXT_texture_border_clamp = 0;
		glExtensions.EXT_sRGB_write_control = 0;
	}
}
#endif

extern int totalAssets;
extern int totalAssetsLoaded;

static void avdl_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority);

extern int dd_flag_exit;

struct avdl_engine engine;

#ifndef AVDL_DIRECT3D11

extern GLuint defaultProgram;
extern GLuint currentProgram;

#include <stdio.h>

extern int avdl_use_default_locale;

int avdl_engine_init(struct avdl_engine *o) {

	// default at 30 FPS
	avdl_engine_setFPS(30);

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	//o->nworld_ready = 0;
	nworld_ready = 0;
	o->nworld_loading = 0;
	o->nworld_size = 0;
	o->nworld_constructor = 0;

	#if defined(AVDL_QUEST2)
	o->resumed = 0;
	o->focused = 0;
	o->NativeWindow = 0;
	o->frameCount = -1;
	o->SessionActive = 0;
	#endif

	avdl_time_start(&o->end_of_update_time);
	avdl_time_start(&o->frame_duration);

	#ifdef AVDL_STEAM
	if (!o->verify) {
		if (!avdl_steam_init()) {
			dd_log("avdl: error initialising steam");
			return -1;
		}
		if (avdl_use_default_locale) {
			avdl_locale_set(avdl_locale_getSystemLocale());
		}
	}
	#endif

	// avdl input system
	avdl_input_Init(&o->input);

	#if defined(AVDL_DIRECT3D11)
	#elif defined(_WIN32) || defined(WIN32)
	const PROJ_LOC_TYPE *proj_loc = avdl_getProjectLocation();
	if (proj_loc) {
		if (_wchdir(proj_loc) != 0) {
			dd_log("avdl: failed to change directory: %lS", _wcserror(errno));
			return -1;
		}
	}
	else {
		dd_log("avdl error: unable to get project location");
	}
	#endif

	o->achievements = avdl_achievements_create();
	avdl_assetManager_init();

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	/*
	// initialise curl
	curl_global_init(CURL_GLOBAL_ALL);

	if (pthread_mutex_init(&asyncCallMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for async calls init failed");
		return -1;
	}
	*/
	#endif

	// default anti-aliasing 8 samples
	avdl_graphics_SetMSAntiAlias(8);

	// start at full volume
	avdl_sound_setVolume(100);

	// initialise pre-game data to defaults then to game-specifics
	dd_gameInitDefault();
	dd_gameInit();

	// audio
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	if (o->verify) {
		dd_hasAudio = 0;
	}

	// audio is meant to be active
	if (dd_hasAudio) {

		/*
		 * initialise audio, if it fails, don't play audio at all
		 * during the game
		 */
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
			dd_log("avdl: error initialising audio mixer");
			dd_hasAudio = 0;
		}
		// audio initialisation succeeded
		else {
			Mix_Init(MIX_INIT_OGG);

			// make sure there's at least 8 channels
			dd_numberOfAudioChannels = Mix_AllocateChannels(-1);
			if (dd_numberOfAudioChannels < 8) {
				dd_numberOfAudioChannels = Mix_AllocateChannels(8);
			}

		}

	} // init audio

	// audio is off - display message about it
	if (!dd_hasAudio && !o->verify) {
		dd_log("avdl error: Game will play without audio");
	}

	#endif

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	avdl_graphics_generateContext();
	#endif

	// window
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	srand(time(NULL));

	if (!o->verify) {
		if (avdl_graphics_Init()) {
			dd_log("avdl: error initialising engine graphics");
			return -1;
		}

		if (avdl_graphics_CreateWindow(&o->graphics) != 0) {
			dd_log("avdl: error creating window");
			return -1;
		}
	}
	if (avdl_engine_hasVSync()) {
		o->avdl_vsync = -1;
		avdl_engine_setVSync(1);
	}
	else {
		o->avdl_vsync = 0;
	}

	//handleResize(dd_window_width(), dd_window_height());

	#endif

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS ) || defined(AVDL_QUEST2)
	avdl_engine_resize(o, dd_window_width(), dd_window_height());
	#endif

	/*
	 * string3d initialisation for displaying text
	 */
	avdl_font_init();

	#if defined(AVDL_QUEST2)

	// opengl extensions
	EglInitExtensions();
	if (!glExtensions.EXT_sRGB_write_control) {
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	}

	// VR view matrices (2 view + 2 proj - one for each eye)
	glGenBuffers(1, &o->SceneMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, o->SceneMatrices);
	glBufferData(
		GL_UNIFORM_BUFFER,
		4 * sizeof(struct dd_matrix),
		0,
		GL_STATIC_DRAW
	);
	GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	// framebuffer
	PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR =
		(PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)eglGetProcAddress(
			"glFramebufferTextureMultiviewOVR"
		);
	PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR =
		(PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)eglGetProcAddress(
			"glFramebufferTextureMultisampleMultiviewOVR"
		);

	o->Elements = malloc(sizeof(struct Element) *o->SwapChainLength);

	for (int i = 0; i < o->SwapChainLength; i++) {
		struct Element *el = &o->Elements[i];

		// create color buffer texture
		el->ColorTexture = o->colorTextures[i];
		GLenum colorTextureTarget = GL_TEXTURE_2D_ARRAY;
		glBindTexture(colorTextureTarget, el->ColorTexture);
		glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glTexParameterfv(colorTextureTarget, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(colorTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(colorTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(colorTextureTarget, 0);

		// create depth buffer texture
		glGenTextures(1, &el->DepthTexture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, el->DepthTexture);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, dd_width, dd_height, 2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		// create frame buffer
		glGenFramebuffers(1, &el->FrameBufferObject);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, el->FrameBufferObject);
		if (o->Multisamples > 1 && (glFramebufferTextureMultisampleMultiviewOVR != 0)) {
			glFramebufferTextureMultisampleMultiviewOVR(
				GL_DRAW_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				el->DepthTexture,
				0,
				o->Multisamples,
				0,
				2
			);
			glFramebufferTextureMultisampleMultiviewOVR(
				GL_DRAW_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				el->ColorTexture,
				0,
				o->Multisamples,
				0,
				2
			);
		} else {
			glFramebufferTextureMultiviewOVR(
				GL_DRAW_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				el->DepthTexture,
				0,
				0,
				2
			);
			glFramebufferTextureMultiviewOVR(
				GL_DRAW_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				el->ColorTexture,
				0,
				0,
				2
			);
		}

		GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
			dd_log("avdl error: could not get framebuffer status");
			return -1;
		}
	}

	#endif

	return 0;
}
#endif

int avdl_engine_clean(struct avdl_engine *o) {

	#ifdef AVDL_DIRECT3D11
	#else
	avdl_achievements_clean(o->achievements);
	avdl_assetManager_deinit();

	if (o->cworld) {
		o->cworld->clean(o->cworld);
		free(o->cworld);
		o->cworld = 0;
	}

	avdl_font_deinit();

	// destroy window
	avdl_graphics_DestroyWindow(&o->graphics);

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	Mix_Quit();
	SDL_Quit();
	#endif

	#endif

	#ifdef AVDL_STEAM
	if (!o->verify) {
		avdl_steam_shutdown();
	}
	#endif

	#if defined(AVDL_QUEST2)
	glDeleteBuffers(1, &o->SceneMatrices);

	for (int i = 0; i < o->SwapChainLength; i++) {
		struct Element *el = &o->Elements[i];
		glDeleteFramebuffers(1, &el->FrameBufferObject);
		glDeleteTextures(1, &el->DepthTexture);
	}
	free(o->Elements);
	o->Elements = 0;

	xrDestroySession(o->session);
	xrDestroySpace(o->HeadSpace);
	xrDestroySpace(o->LocalSpace);
	#endif

	return 0;
}

#ifndef AVDL_DIRECT3D11
int avdl_engine_draw(struct avdl_engine *o) {

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	if (dd_flag_exit) {
		return 0;
	}
	#endif

	// gamma correction - this is not needed in OpenGL ES
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	GL(glEnable(GL_FRAMEBUFFER_SRGB));
	#endif

	GL(glEnable(GL_CULL_FACE));
	GL(glCullFace(GL_BACK));

	#if defined(AVDL_QUEST2)

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Render the eye images.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, o->Elements[o->swapChainIndex].FrameBufferObject);

	glEnable(GL_SCISSOR_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//GL(glEnable(GL_CULL_FACE));
	glDisable(GL_CULL_FACE);
	//GL(glCullFace(GL_BACK));
	glDisable(GL_BLEND);
	glViewport(0, 0, dd_window_width(), dd_window_height());
	glScissor(0, 0, dd_window_width(), dd_window_height());

	glUseProgram(defaultProgram);

	// update controller matrices
	for (int i = 0; i < 2; ++i) {
		if (o->RenderController[i]) {
			dd_matrix_setControllerMatrix(i, &o->ControllerPoses[i]);
			dd_matrix_setControllerVisible(i, 1);
		}
	}

	// multiview implementation
	GLuint viewIdLoc = glGetUniformLocation(currentProgram, "ViewID");
	if (viewIdLoc >= 0) {
		glUniform1i(viewIdLoc, 0);
	}

	// Update the scene matrices.
	glBindBuffer(GL_UNIFORM_BUFFER, o->SceneMatrices);
	struct dd_matrix* sceneMatrices = (struct dd_matrix*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		4 * sizeof(struct dd_matrix),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);

	if (sceneMatrices != 0) {
		memcpy((char*)sceneMatrices, &o->View, 4 * sizeof(struct dd_matrix));
	}

	// eye matrices
	GLuint sceneMatricesLoc = glGetUniformBlockIndex(currentProgram, "SceneMatrices");
	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		sceneMatricesLoc,
		o->SceneMatrices
	);
	#endif

	// clear everything
	avdl_graphics_ClearToColour();
	avdl_graphics_ClearColourAndDepth();

	// reset view
	dd_matrix_globalInit();

	// draw world
	if (o->cworld && o->cworld->draw) {
		o->cworld->draw(o->cworld);
	}

	avdl_graphics_SwapFramebuffer(&o->graphics);

	#if defined(AVDL_QUEST2)
	GL(glUseProgram(0));
	GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
	#endif

	return 0;
}
#endif

int avdl_engine_isPaused(struct avdl_engine *o) {
	return o->isPaused;
}

void avdl_engine_setPaused(struct avdl_engine *o, int state) {
	o->isPaused = state;
}

int avdl_engine_initWorld(struct avdl_engine *o, void (*constructor)(struct dd_world*), int size) {
	nworld_constructor = constructor;
	nworld_size = size;
	//o->nworld_constructor = constructor;
	//o->nworld_size = size;
	//o->nworld_ready = 1;
	nworld_ready = 1;
	return 0;
}

struct dd_matrix matPerspective;

int avdl_engine_resize(struct avdl_engine *o, int w, int h) {

	avdl_graphics_Viewport(0, 0, w, h);

	int ypriority;
	if (w > h) {
		dd_fovaspect_set((double) w /(double) h);
		ypriority = 1;
	}
	else {
		dd_fovaspect_set((double) h /(double) w);
		ypriority = 0;
	}

	// perspective projection matrix
	avdl_perspective((float *)&matPerspective, dd_fovy_get(), dd_fovaspect_get(), 1.0, 200.0, ypriority);

	if (o->cworld && o->cworld->resize) {
		o->cworld->resize(o->cworld);
	}

	return 0;
}

int avdl_engine_update(struct avdl_engine *o, float dt) {

	avdl_time_end(&o->end_of_update_time);
	float dt2 = avdl_time_getTimeDouble(&o->end_of_update_time);
	avdl_time_start(&o->end_of_update_time);

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	if (avdl_engine_isPaused(o)) {
		return 0;
	}
	#endif

	/*
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	// handle asynchronous calls
	if (dd_isAsyncCallActive) {
		pthread_mutex_lock(&asyncCallMutex);
		if (dd_asyncCall.isComplete) {
			dd_isAsyncCallActive = 0;
			if (dd_asyncCall.callback) {
				dd_asyncCall.callback(dd_asyncCall.context);
			}
		}
		pthread_mutex_unlock(&asyncCallMutex);
	}
	#endif
	*/


	#ifdef AVDL_STEAM
	avdl_steam_update();
	#endif

	// a new world is signaled to be loaded
	if (nworld_constructor) {
	//if (o->nworld_constructor)

		int hasAssetManagerLoaded = 0;
		if (totalAssets == 0) {
			hasAssetManagerLoaded = 1;
		}
		else {
			if (totalAssetsLoaded == totalAssets) {
				hasAssetManagerLoaded = 1;
			}
		}

		// temp
		o->nworld_constructor = nworld_constructor;
		o->nworld_size = nworld_size;

		// the new world has not started loading, so start loading it
		if (!o->nworld_loading) {

			// set flag that world is loading
			o->nworld_loading = 1;

			// clear everything loading on asset manager
			avdl_assetManager_clear();

			// allocate new world and construct it
			o->nworld = malloc(o->nworld_size);
			o->nworld_constructor(o->nworld);

			// from now on, loading new assets is not allowed
			avdl_assetManager_lockLoading();

		}
		else
		// The world has finished loading
		//if (o->nworld && o->nworld_ready && avdl_assetManager_getLoadedProportion() >= 1.0)
		if (o->nworld && nworld_ready && hasAssetManagerLoaded) {

			/*
			// Cancel async calls
			dd_isAsyncCallActive = 0;
			*/

			// free any previous world
			if (o->cworld) {
				o->cworld->clean(o->cworld);
				free(o->cworld);
				o->cworld = 0;
			}

			// from now on, new assets can be loaded again
			avdl_assetManager_unlockLoading();

			// Apply the new world
			o->cworld = o->nworld;
			o->nworld = 0;

			// from this point on, new world can be set
			o->nworld_constructor = 0;
			nworld_constructor = 0;
			o->nworld_size = 0;
			//o->nworld_ready = 0;
			nworld_ready = 0;
			o->nworld_loading = 0;

			// notify the world that it has loaded assets
			o->cworld->onload(o->cworld);

			// resize the new world
			if (o->cworld->resize) {
				o->cworld->resize(o->cworld);
			}

		}

	}

	avdl_input_update(&o->input);

	// handle key input
	if (o->cworld && o->cworld->key_input && o->input.input_key) {
		o->cworld->key_input(o->cworld, o->input.input_key);
		o->input.input_key = 0;
	}

	// handle mouse input
	if (o->cworld && o->cworld->mouse_input && avdl_input_GetInputTotal(&o->input) > 0) {
		int totalInput = avdl_input_GetInputTotal(&o->input);
		for (int i = 0; i < totalInput; i++) {
			o->cworld->mouse_input(o->cworld, avdl_input_GetButton(&o->input, i), avdl_input_GetState(&o->input, i));
		}
		avdl_input_ClearInput(&o->input);
	}

	// update world
	if (o->cworld && o->cworld->update) {
		o->cworld->update(o->cworld, dt2);
	}

	// asset loader will load any new assets
	if (avdl_assetManager_hasAssetsToLoad() && !avdl_assetManager_isLoading()) {
		avdl_assetManager_loadAll();
	}

	return 0;
}

void avdl_perspective(float *matrix, float fovyDegrees, float aspectRatio,
	float znear, float zfar, int ypriority) {

	float ymax, xmax;
	if (ypriority) {
		#ifdef AVDL_DIRECT3D11
		ymax = znear * tanf(fovyDegrees * 3.14 / 360.0);
		#else
		ymax = znear * tanf(fovyDegrees * M_PI / 360.0);
		#endif
		xmax = ymax * aspectRatio;
	}
	else {
		#ifdef AVDL_DIRECT3D11
		xmax = znear * tanf(fovyDegrees * 3.14 / 360.0);
		#else
		xmax = znear * tanf(fovyDegrees * M_PI / 360.0);
		#endif
		ymax = xmax * aspectRatio;
	}

	float left = -xmax;
	float right = xmax;
	float bottom = -ymax;
	float top = ymax;

	float temp, temp2, temp3, temp4;
	temp = 2.0 * znear;
	temp2 = right - left;
	temp3 = top - bottom;
	temp4 = zfar - znear;
	#ifdef AVDL_DIRECT3D11
	matrix[ 0] = temp / temp2;
	matrix[ 4] = 0.0;
	matrix[ 8] = 0.0;
	matrix[12] = 0.0;
	matrix[ 1] = 0.0;
	matrix[ 5] = temp / temp3;
	matrix[ 9] = 0.0;
	matrix[13] = 0.0;
	matrix[ 2] = (right + left) / temp2;
	matrix[ 6] = (top + bottom) / temp3;
	matrix[10] = (-zfar - znear) / temp4;
	matrix[14] = -1.0;
	matrix[ 3] = 0.0;
	matrix[ 7] = 0.0;
	matrix[11] = (-temp * zfar) / temp4;
	matrix[15] = 0.0;
	#else
	matrix[0] = temp / temp2;
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.0;
	matrix[4] = 0.0;
	matrix[5] = temp / temp3;
	matrix[6] = 0.0;
	matrix[7] = 0.0;
	matrix[8] = (right + left) / temp2;
	matrix[9] = (top + bottom) / temp3;
	matrix[10] = (-zfar - znear) / temp4;
	matrix[11] = -1.0;
	matrix[12] = 0.0;
	matrix[13] = 0.0;
	matrix[14] = (-temp * zfar) / temp4;
	matrix[15] = 0.0;

	#endif
}

#ifndef AVDL_DIRECT3D11
static void handleMousePress(struct avdl_engine *o, int button, int state, int x, int y) {

	// SDL to AVDL conversion
	int state_temp = 0;
	switch (state) {
		//case GLUT_DOWN:
		case 0:
			state_temp = DD_INPUT_MOUSE_TYPE_PRESSED;
			break;
		//case GLUT_UP:
		case 1:
			state_temp = DD_INPUT_MOUSE_TYPE_RELEASED;
			break;
	}

	int button_temp = 0;
	switch (button) {
		//case GLUT_LEFT_BUTTON:
		case 0:
			button_temp = DD_INPUT_MOUSE_BUTTON_LEFT;
			break;
		//case GLUT_MIDDLE_BUTTON:
		case 1:
			button_temp = DD_INPUT_MOUSE_BUTTON_MIDDLE;
			break;
		//case GLUT_RIGHT_BUTTON:
		case 2:
			button_temp = DD_INPUT_MOUSE_BUTTON_RIGHT;
			break;
	}
	avdl_input_AddInput(&o->input, button_temp, state_temp, x, y);

}

int avdl_engine_loop(struct avdl_engine *o) {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )

	if (o->verify) {
		avdl_engine_verify(o);
		return 0;
	}

	int isRunning = 1;
	SDL_Event event;
	while (isRunning && !dd_flag_exit) {

		avdl_time_start(&o->frame_duration);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				isRunning = 0;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					avdl_engine_resize(o, event.window.data1, event.window.data2);
				}
				break;
			case SDL_MOUSEMOTION:
				avdl_input_AddPassiveMotion(&o->input, event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				handleMousePress(o, 0, 0, event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONUP:
				handleMousePress(o, 0, 1, event.motion.x, event.motion.y);
				break;
			case SDL_KEYDOWN:
				// temporary keyboard controls
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					o->input.input_key = 27;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_A) {
					o->input.input_key = 97;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_D) {
					o->input.input_key = 100;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_W) {
					o->input.input_key = 119;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_S) {
					o->input.input_key = 115;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					o->input.input_key = 32;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					o->input.input_key = 13;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					o->input.input_key = 1;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
					o->input.input_key = 2;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					o->input.input_key = 3;
				}
				else
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					o->input.input_key = 4;
				}
				break;
			}
		}

		//update();
		avdl_engine_update(o, 1);

		// prepare next frame
		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		avdl_engine_draw(o);
		//draw();
		#endif

		avdl_time_end(&o->frame_duration);
		if (o->avdl_fps > 0) {
			float frame_duration_ms = avdl_time_getTimeDouble(&o->frame_duration) *1000;
			// frame completed within the expected time
			if (frame_duration_ms < o->avdl_fps_delay) {
				SDL_Delay(o->avdl_fps_delay -frame_duration_ms);
			}
			// frame took longer to complete than it should
			// for now just proceed to the next frame immidiatelly
			else {
			}
		}
	}
	#endif
	return 0;
}
#endif

void avdl_engine_verify(struct avdl_engine *o) {
	/*
	//avdl_assetManager_lockLoading();

	// allocate new world and construct it
	nworld = malloc(nworld_size);
	nworld_constructor(nworld);

	// from now on, new assets can be loaded again
	//avdl_assetManager_unlockLoading();

	// Apply the new world
	cworld = nworld;
	nworld = 0;

	// notify the world that it has loaded assets
	cworld->onload(cworld);

	// resize the new world
	if (cworld->resize) {
		cworld->resize(cworld);
	}

	cworld->update(cworld);
	*/
}

void avdl_engine_setFPS(int fps) {

	// 0 not allowed
	if (fps == 0) {
		fps = 1;
	}
	else
	// -1 is as fast as possible
	if (fps < 0) {
		fps = -1;
	}
	engine.avdl_fps = fps;
	if (engine.avdl_fps > 0) {
		engine.avdl_fps_delay = 1000/engine.avdl_fps;
	}
}

void avdl_engine_setVSync(int flag) {

	// skip if already set or vsync not supported
	if (engine.avdl_vsync == flag || !avdl_engine_hasVSync()) {
		return;
	}

	// try to set vsync for given flat, if failed, assume no vsync is available
	if (avdl_graphics_setVSync(flag) != 0) {
		engine.avdl_vsync = 0;
	}
	// vsync is available
	else {
		engine.avdl_vsync = flag;
	}
}

// which platforms support changing vsync
int avdl_engine_hasVSync() {
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	return 1;
	#else
	return 0;
	#endif
}
