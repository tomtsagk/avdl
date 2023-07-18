#include "avdl_engine.h"

#include "dd_log.h"

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

#if defined(AVDL_QUEST2)
#include <unistd.h>
#include <sys/prctl.h>

#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1

#include <openxr/fb_scene_capture.h>

#include <OVR_Math.h>
#include <OVR_Platform.h>

#include "avdl_cengine.h"

#include <android_native_app_glue.h>
void avdl_handle_cmd(struct android_app* androidApp, int32_t cmd) {
	struct avdl_engine *engine = (struct avdl_engine*)androidApp->userData;

	switch (cmd) {
	case APP_CMD_START:
		//dd_log("onStart()");
		break;
	case APP_CMD_RESUME:
		//dd_log("onResume()");
		engine->resumed = 1;
		break;
	case APP_CMD_PAUSE:
		//dd_log("onPause()");
		engine->resumed = 0;
		break;
	case APP_CMD_STOP:
		//dd_log("onStop()");
		break;
	case APP_CMD_DESTROY:
		//dd_log("onDestroy()");
		engine->NativeWindow = 0;
		break;
	case APP_CMD_INIT_WINDOW:
		//dd_log("surfaceCreated()");
		engine->NativeWindow = androidApp->window;
		break;
	case APP_CMD_TERM_WINDOW:
		//dd_log("surfaceDestroyed()");
		engine->NativeWindow = 0;
		break;
	}
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <jni.h>
extern "C" {
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass *clazz;
extern pthread_mutex_t jniMutex;
extern jmethodID BitmapMethodId;
}
#endif

using namespace OVR;

#if !defined(EGL_OPENGL_ES3_BIT_KHR)
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

extern int dd_flag_exit;

extern "C" struct avdl_engine engine;

// Forward declarations
XrInstance instance;

extern "C" void set_android_save_dir(jobject activity);
extern void avdl_handle_cmd(struct android_app* androidApp, int32_t cmd);

void android_main(struct android_app* androidApp) {

	// egl initialisation
	EGLint eglMajorVersion = 0;
	EGLint eglMinorVersion = 0;
	EGLDisplay eglDisplay = 0;
	EGLConfig eglConfig = 0;
	EGLSurface eglTinySurface = EGL_NO_SURFACE;
	EGLContext eglContext = EGL_NO_CONTEXT;

	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion);
	EGLint numConfigs = 0;
	eglGetConfigs(eglDisplay, 0, 0, &numConfigs);
	EGLConfig *configs = (EGLConfig *) malloc(sizeof(EGLConfig) *numConfigs);
	if (eglGetConfigs(eglDisplay, configs, numConfigs, &numConfigs) == EGL_FALSE) {
		dd_log("avdl error: could not get configs");
		exit(-1);
	}
	const EGLint configAttribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 0,
		EGL_STENCIL_SIZE, 0,
		EGL_SAMPLES, 0,
		EGL_NONE,
	};

	// find the desired config
	for (int i = 0; i < numConfigs; i++) {
		EGLint value = 0;

		eglGetConfigAttrib(eglDisplay, configs[i], EGL_RENDERABLE_TYPE, &value);
		if ((value & EGL_OPENGL_ES3_BIT_KHR) != EGL_OPENGL_ES3_BIT_KHR) {
			continue;
		}

		eglGetConfigAttrib(eglDisplay, configs[i], EGL_SURFACE_TYPE, &value);
		if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) {
			continue;
		}

		int j = 0;
		for (; configAttribs[j] != EGL_NONE; j += 2) {
			eglGetConfigAttrib(eglDisplay, configs[i], configAttribs[j], &value);
			if (value != configAttribs[j + 1]) {
				break;
			}
		}

		// found config
		if (configAttribs[j] == EGL_NONE) {
			eglConfig = configs[i];
			break;
		}
	}
	free(configs);

	if (eglConfig == 0) {
		dd_log("avdl error: could not find suitable configuration");
		exit(-1);
	}

	// create egl context
	EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
	eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
	if (eglContext == EGL_NO_CONTEXT) {
		dd_log("avdl error: unable to create egl context");
		exit(-1);
	}

	// configure surface
	const EGLint surfaceAttribs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
	eglTinySurface = eglCreatePbufferSurface(eglDisplay, eglConfig, surfaceAttribs);
	if (eglTinySurface == EGL_NO_SURFACE) {
		dd_log("avdl error: unable to create egl surface");
		eglDestroyContext(eglDisplay, eglContext);
		eglContext = EGL_NO_CONTEXT;
		exit(-1);
	}

	// make egl display to draw on
	if (eglMakeCurrent(eglDisplay, eglTinySurface, eglTinySurface, eglContext) == EGL_FALSE) {
		dd_log("avdl error: unable to make egl display");
		eglDestroySurface(eglDisplay, eglTinySurface);
		eglDestroyContext(eglDisplay, eglContext);
		eglContext = EGL_NO_CONTEXT;
		exit(-1);
	}

	/*
	// initialise pthread mutex for jni
	if (pthread_mutex_init(&jniMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for jni init failed");
		return;
	}
	*/

	// attach to jni
	(*androidApp->activity->vm).AttachCurrentThread(&jniEnv, nullptr);
	prctl(PR_SET_NAME, (long)"avdl main", 0, 0, 0);

	// get save location
	set_android_save_dir(androidApp->activity->clazz);

	#if defined(AVDL_OCULUS)
	// meta oculus platform sdk
	// at some point try "ovr_PlatformInitializeAndroidAsynchronous" for async
	if (ovr_PlatformInitializeAndroid(AVDL_OCULUS_PROJECT_ID, androidApp->activity->clazz, jniEnv) != ovrPlatformInitialize_Success) {
		// should exit
		dd_log("avdl error: cannot initiale oculus platform sdk");
		exit(-1);
	}
	ovr_Entitlement_GetIsViewerEntitled();
	#endif

	// Initialise LoaderXR loader on Android
	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);
	if (xrInitializeLoaderKHR == NULL) {
		dd_log("avdl error: cannot initialise OpenXR Loader");
		exit(-1);
	}

	XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid;
	memset(&loaderInitializeInfoAndroid, 0, sizeof(loaderInitializeInfoAndroid));
	loaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
	loaderInitializeInfoAndroid.next = NULL;
	loaderInitializeInfoAndroid.applicationVM = androidApp->activity->vm;
	loaderInitializeInfoAndroid.applicationContext = androidApp->activity->clazz;
	xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid);

	// required extensions for Quest2
	const char* const requiredExtensionNames[] = {
		XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME,
		XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME,
		XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME,
		XR_FB_SPATIAL_ENTITY_EXTENSION_NAME,
		XR_FB_SPATIAL_ENTITY_QUERY_EXTENSION_NAME,
		XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME,
		XR_FB_SPATIAL_ENTITY_CONTAINER_EXTENSION_NAME,
		XR_FB_SCENE_EXTENSION_NAME,
		XR_FB_SCENE_CAPTURE_EXTENSION_NAME
	};
	const uint32_t numRequiredExtensions = sizeof(requiredExtensionNames) / sizeof(requiredExtensionNames[0]);

	// confirm extensions
	XrResult xrResult;
	PFN_xrEnumerateInstanceExtensionProperties xrEnumerateInstanceExtensionProperties;
	xrResult = xrGetInstanceProcAddr(
		XR_NULL_HANDLE,
		"xrEnumerateInstanceExtensionProperties",
		(PFN_xrVoidFunction*)&xrEnumerateInstanceExtensionProperties
	);
	if (xrResult != XR_SUCCESS) {
		dd_log("avdl error: `xrEnumerateInstanceExtensionProperties` not found");
		exit(-1);
	}

	// get number of extensions
	uint32_t numInputExtensions = 0;
	uint32_t numOutputExtensions = 0;
	xrEnumerateInstanceExtensionProperties(NULL, numInputExtensions, &numOutputExtensions, NULL);
	numInputExtensions = numOutputExtensions;

	// prepare array to save extension data
	XrExtensionProperties *extensionProperties = (XrExtensionProperties *)malloc(sizeof(struct XrExtensionProperties) *numOutputExtensions);
	for (uint32_t i = 0; i < numOutputExtensions; i++) {
		extensionProperties[i].type = XR_TYPE_EXTENSION_PROPERTIES;
		extensionProperties[i].next = NULL;
	}

	// get extensions
	xrEnumerateInstanceExtensionProperties(NULL, numInputExtensions, &numOutputExtensions, extensionProperties);

	// confirm required extensions
	for (uint32_t i = 0; i < numRequiredExtensions; i++) {
		int found = 0;
		for (uint32_t j = 0; j < numOutputExtensions; j++) {
			if (strcmp(requiredExtensionNames[i], extensionProperties[j].extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			dd_log("avdl error: could not find extension %s", requiredExtensionNames[i]);
			exit(-1);
		}
	}

	// clean up extension properties
	free(extensionProperties);

	// create OpenXR instance
	XrApplicationInfo appInfo = {};
	strcpy(appInfo.applicationName, "Avdl Project");
	appInfo.applicationVersion = 0;
	strcpy(appInfo.engineName, "Avdl");
	appInfo.engineVersion = 0;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	XrInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.next = nullptr;
	instanceCreateInfo.createFlags = 0;
	instanceCreateInfo.applicationInfo = appInfo;
	instanceCreateInfo.enabledApiLayerCount = 0;
	instanceCreateInfo.enabledApiLayerNames = NULL;
	instanceCreateInfo.enabledExtensionCount = numRequiredExtensions;
	instanceCreateInfo.enabledExtensionNames = requiredExtensionNames;

	XrResult initResult;
	initResult = xrCreateInstance(&instanceCreateInfo, &instance);
	if (initResult != XR_SUCCESS) {
		dd_log("avdl error: unable to create XR instance: %d.", initResult);
		exit(-1);
	}

	// headset VR only for now
	XrSystemGetInfo systemGetInfo = {};
	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemGetInfo.next = NULL;
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	// get system id
	initResult = xrGetSystem(instance, &systemGetInfo, &engine.SystemId);
	if (initResult != XR_SUCCESS) {
		dd_log("avdl_error: cannot get system");
		exit(-1);
	}

	// system properties
	XrSystemProperties systemProperties = {};
	systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	xrGetSystemProperties(instance, engine.SystemId, &systemProperties);

	// project needs more layers than allowed
	if (systemProperties.graphicsProperties.maxLayerCount < ovrMaxLayerCount) {
		dd_log("avdl error: system has less available layers than expected");
		exit(-1);
	}

	// get graphics requirements
	PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnGetOpenGLESGraphicsRequirementsKHR = NULL;
	xrResult = xrGetInstanceProcAddr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&pfnGetOpenGLESGraphicsRequirementsKHR));
	if (xrResult != XR_SUCCESS) {
		dd_log("avdl error: unable to find `xrGetOpenGLESGraphicsRequirementsKHR`");
		exit(-1);
	}
	XrGraphicsRequirementsOpenGLESKHR graphicsRequirements = {};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR;
	pfnGetOpenGLESGraphicsRequirementsKHR(instance, engine.SystemId, &graphicsRequirements);

	// check OpenGLES requirements
	int eglMajor = 0;
	int eglMinor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &eglMajor);
	glGetIntegerv(GL_MINOR_VERSION, &eglMinor);
	const XrVersion eglVersion = XR_MAKE_VERSION(eglMajor, eglMinor, 0);
	if (eglVersion < graphicsRequirements.minApiVersionSupported ||
	    eglVersion > graphicsRequirements.maxApiVersionSupported) {
		dd_log("avdl error: OpenGLES version %d.%d not supported", eglMajor, eglMinor);
		exit(-1);
	}

	int main_thread_id = gettid();

	// create OpenXR session with opengles
	XrGraphicsBindingOpenGLESAndroidKHR graphicsBindingAndroidGLES = {};
	graphicsBindingAndroidGLES.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR;
	graphicsBindingAndroidGLES.next = NULL;
	graphicsBindingAndroidGLES.display = eglDisplay;
	graphicsBindingAndroidGLES.config = eglConfig;
	graphicsBindingAndroidGLES.context = eglContext;

	// create session
	XrSessionCreateInfo sessionCreateInfo = {};
	sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next = &graphicsBindingAndroidGLES;
	sessionCreateInfo.createFlags = 0;
	sessionCreateInfo.systemId = engine.SystemId;
	xrResult = xrCreateSession(instance, &sessionCreateInfo, &engine.session);
	if (xrResult != XR_SUCCESS) {
		dd_log("avdl error: cannot create XR session: %d", initResult);
		exit(-1);
	}

	// avdl supports VR only on Quest 2 for now, so only stereo view supported
	const XrViewConfigurationType supportedViewConfigType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	// get number of supported view configurations
	uint32_t viewportConfigTypeCount = 0;
	xrEnumerateViewConfigurations(instance, engine.SystemId, 0, &viewportConfigTypeCount, NULL);

	// get all configuration types
	XrViewConfigurationType *viewportConfigurationTypes = (XrViewConfigurationType *)malloc(sizeof(XrViewConfigurationType) *viewportConfigTypeCount);
	xrEnumerateViewConfigurations(
		instance,
		engine.SystemId,
		viewportConfigTypeCount,
		&viewportConfigTypeCount,
		viewportConfigurationTypes
	);

	int foundDesiredView = 0;
	for (uint32_t i = 0; i < viewportConfigTypeCount; i++) {
		const XrViewConfigurationType viewportConfigType = viewportConfigurationTypes[i];

		// not a supported view config, skip
		if (viewportConfigType != supportedViewConfigType) {
			continue;
		}

		foundDesiredView = 1;

		// grab the configuration of the view
		XrViewConfigurationProperties viewportConfig = {XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
		xrGetViewConfigurationProperties(
			instance, engine.SystemId, viewportConfigType, &viewportConfig
		);

		// get number of views (should be 2 for the quest 2)
		uint32_t viewCount;
		xrEnumerateViewConfigurationViews(
			instance, engine.SystemId, viewportConfigType, 0, &viewCount, NULL
		);

		// only 2 views supported for now
		if (viewCount == 2) {

			// init configuration view elements
			XrViewConfigurationView *elements = (XrViewConfigurationView *) malloc(sizeof(XrViewConfigurationView) *viewCount);
			for (uint32_t e = 0; e < viewCount; e++) {
				elements[e].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
				elements[e].next = NULL;
			}

			// grab view configurations
			xrEnumerateViewConfigurationViews(
				instance, engine.SystemId, viewportConfigType, viewCount, &viewCount, elements
			);

			// cache view config properties
			for (uint32_t e = 0; e < viewCount; e++) {
				engine.ViewConfigurationView[e] = elements[e];
			}

			free(elements);
		}
		else {
			dd_log("avdl error: only supports configuration with 2 views");
		}
		break;
	}

	free(viewportConfigurationTypes);

	// Get the viewport configuration info for the chosen viewport configuration type.
	engine.ViewportConfig.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;

	xrGetViewConfigurationProperties(instance, engine.SystemId, supportedViewConfigType, &engine.ViewportConfig);

	// spaces
	/*
	uint32_t numOutputSpaces = 0;
	xrEnumerateReferenceSpaces(app.Session, 0, &numOutputSpaces, NULL);

	auto referenceSpaces = new XrReferenceSpaceType[numOutputSpaces];
	xrEnumerateReferenceSpaces(app.Session, numOutputSpaces, &numOutputSpaces, referenceSpaces);

	delete[] referenceSpaces;
	*/

	// create head space
	XrReferenceSpaceCreateInfo spaceCreateInfo = {};
	spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
	xrCreateReferenceSpace(engine.session, &spaceCreateInfo, &engine.HeadSpace);

	// create local space
	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	xrCreateReferenceSpace(engine.session, &spaceCreateInfo, &engine.LocalSpace);

	// prepare projections
	XrView *projections = (XrView *) malloc(sizeof(XrView) *NUM_EYES);
	for (int eye = 0; eye < NUM_EYES; eye++) {
		projections[eye] = XrView{XR_TYPE_VIEW};
	}

	// save recommended dimensions
	int width = engine.ViewConfigurationView[0].recommendedImageRectWidth;
	int height = engine.ViewConfigurationView[0].recommendedImageRectHeight;

	// prepare swapchain info
	XrSwapchainCreateInfo swapChainCreateInfo = {};
	swapChainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
	swapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.format = GL_SRGB8_ALPHA8;
	swapChainCreateInfo.sampleCount = 1;
	swapChainCreateInfo.width = width;
	swapChainCreateInfo.height = height;
	swapChainCreateInfo.faceCount = 1;
	swapChainCreateInfo.arraySize = 2;
	swapChainCreateInfo.mipCount = 1;

	// create the swapchain
	xrCreateSwapchain(engine.session, &swapChainCreateInfo, &engine.ColorSwapChain);

	// get swapchain images
	xrEnumerateSwapchainImages(engine.ColorSwapChain, 0, &engine.SwapChainLength, nullptr);
	XrSwapchainImageOpenGLESKHR *images = (XrSwapchainImageOpenGLESKHR *) malloc(sizeof(XrSwapchainImageOpenGLESKHR) *engine.SwapChainLength);
	for (uint32_t i = 0; i < engine.SwapChainLength; i++) {
		images[i] = {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR};
	}

	xrEnumerateSwapchainImages(
		engine.ColorSwapChain,
		engine.SwapChainLength,
		&engine.SwapChainLength,
		(XrSwapchainImageBaseHeader*)images
	);

	// textures to draw onto
	GLuint *colorTextures = (GLuint *) malloc(sizeof(GLuint) *engine.SwapChainLength);
	for (uint32_t i = 0; i < engine.SwapChainLength; i++) {
		colorTextures[i] = GLuint(images[i].image);
	}

	// avdl
	engine.Multisamples = 4;
	engine.colorTextures = colorTextures;
	dd_width = width;
	dd_height = height;
	engine.input.session = engine.session;
	engine.input.instance = instance;
	avdl_engine_init(&engine);
	avdl_engine_initWorld(&engine, dd_default_world_constructor, dd_default_world_size);
	//avdl_state_initialised = 1;

	free(images);
	free(colorTextures);

	engine.colorTextures = 0;

	androidApp->userData = &engine;
	androidApp->onAppCmd = avdl_handle_cmd;

	// main loop
	while (androidApp->destroyRequested == 0 && !dd_flag_exit) {
		engine.frameCount++;

		#if defined(AVDL_OCULUS)
		// check ovr messages
		ovrMessage* message = nullptr;
		while ((message = ovr_PopMessage()) != nullptr) {
			switch (ovr_Message_GetType(message)) {
			case ovrMessage_Entitlement_GetIsViewerEntitled:
				if (ovr_Message_IsError(message)) {
					// user is NOT entitled, exit for now
					dd_log("avdl error: user is not entitled to use software");
					exit(0);
				}

				// user is allowed to use game - continue
				break;
			default:
				// unsupported message
				//dd_log("some other msg: %s", ovr_Message_GetString(message));
				break;
			}
		}
		#endif

		// handle android events
		while (1) {
			int events;
			struct android_poll_source* source;

			// on signal to exit, wait indefinitely for an event, otherwise return instantly
			const int timeoutMilliseconds = (engine.resumed == 0 && engine.SessionActive == false &&
				androidApp->destroyRequested == 0)
				? -1
				: 0;
			if (ALooper_pollAll(timeoutMilliseconds, NULL, &events, (void**)&source) < 0) {
				break;
			}

			// process event as normal
			if (source != NULL) {
				source->process(androidApp, source);
			}
		}

		// XR events
		XrEventDataBuffer eventDataBuffer = {};

		// Poll for events
		while (1) {
			XrEventDataBaseHeader* baseEventHeader = (XrEventDataBaseHeader*)(&eventDataBuffer);
			baseEventHeader->type = XR_TYPE_EVENT_DATA_BUFFER;
			baseEventHeader->next = NULL;
			XrResult r;
			r = xrPollEvent(instance, &eventDataBuffer);
			if (r != XR_SUCCESS) {
				break;
			}

			switch (baseEventHeader->type) {

			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				const XrEventDataSessionStateChanged* session_state_changed_event =
					(XrEventDataSessionStateChanged*)(baseEventHeader);

				switch (session_state_changed_event->state) {

				case XR_SESSION_STATE_FOCUSED:
					engine.focused = 1;
					break;
				case XR_SESSION_STATE_VISIBLE:
					engine.focused = 0;
					break;
				case XR_SESSION_STATE_READY: {
					XrSessionState state = session_state_changed_event->state;

					XrSessionBeginInfo sessionBeginInfo = {};
					sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
					sessionBeginInfo.next = nullptr;
					sessionBeginInfo.primaryViewConfigurationType = engine.ViewportConfig.viewConfigurationType;

					XrResult result;
					result = xrBeginSession(engine.session, &sessionBeginInfo);

					engine.SessionActive = (result == XR_SUCCESS);

					// Set session state once we have entered VR mode and have a valid session object.
					if (engine.SessionActive) {
						XrPerfSettingsLevelEXT cpuPerfLevel = XR_PERF_SETTINGS_LEVEL_SUSTAINED_HIGH_EXT;
						XrPerfSettingsLevelEXT gpuPerfLevel = XR_PERF_SETTINGS_LEVEL_BOOST_EXT;

						PFN_xrPerfSettingsSetPerformanceLevelEXT pfnPerfSettingsSetPerformanceLevelEXT = NULL;
						xrGetInstanceProcAddr(
							instance,
							"xrPerfSettingsSetPerformanceLevelEXT",
							(PFN_xrVoidFunction*)(&pfnPerfSettingsSetPerformanceLevelEXT)
						);

						pfnPerfSettingsSetPerformanceLevelEXT(
							engine.session, XR_PERF_SETTINGS_DOMAIN_CPU_EXT, cpuPerfLevel
						);
						pfnPerfSettingsSetPerformanceLevelEXT(
							engine.session, XR_PERF_SETTINGS_DOMAIN_GPU_EXT, gpuPerfLevel
						);

						PFN_xrSetAndroidApplicationThreadKHR pfnSetAndroidApplicationThreadKHR = NULL;
						xrGetInstanceProcAddr(
							instance,
							"xrSetAndroidApplicationThreadKHR",
							(PFN_xrVoidFunction*)(&pfnSetAndroidApplicationThreadKHR)
						);

						pfnSetAndroidApplicationThreadKHR(
							engine.session, XR_ANDROID_THREAD_TYPE_APPLICATION_MAIN_KHR, main_thread_id
						);
						pfnSetAndroidApplicationThreadKHR(
							engine.session, XR_ANDROID_THREAD_TYPE_RENDERER_MAIN_KHR, 0
						);
					}
				} break;
				case XR_SESSION_STATE_STOPPING: {
					xrEndSession(engine.session);
					engine.SessionActive = 0;
				} break;
				default:
					break;

				}
			} break;

			// unsupported event
			default:
				break;

			}
		}

		// XR event has possibly removed the session
		if (engine.SessionActive == false) {
			continue;
		}

		// wait for XR frame to be ready
		XrFrameWaitInfo waitFrameInfo = {};
		waitFrameInfo.type = XR_TYPE_FRAME_WAIT_INFO;
		waitFrameInfo.next = NULL;

		XrFrameState frameState = {};
		frameState.type = XR_TYPE_FRAME_STATE;
		frameState.next = NULL;

		xrWaitFrame(engine.session, &waitFrameInfo, &frameState);

		// prepare frame
		XrFrameBeginInfo beginFrameDesc = {};
		beginFrameDesc.type = XR_TYPE_FRAME_BEGIN_INFO;
		beginFrameDesc.next = NULL;
		xrBeginFrame(engine.session, &beginFrameDesc);

		XrSpaceLocation loc = {};
		loc.type = XR_TYPE_SPACE_LOCATION;
		xrLocateSpace(engine.HeadSpace, engine.LocalSpace, frameState.predictedDisplayTime, &loc);
		XrPosef xfLocalFromHead = loc.pose;

		avdl_engine_update(&engine, 0);

		XrViewState viewState = {XR_TYPE_VIEW_STATE, NULL};

		XrViewLocateInfo projectionInfo = {};
		projectionInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
		projectionInfo.viewConfigurationType = engine.ViewportConfig.viewConfigurationType;
		projectionInfo.displayTime = frameState.predictedDisplayTime;
		projectionInfo.space = engine.HeadSpace;

		uint32_t projectionCapacityInput = NUM_EYES;
		uint32_t projectionCountOutput = projectionCapacityInput;

		xrLocateViews(engine.session, &projectionInfo, &viewState, projectionCapacityInput,
			&projectionCountOutput, projections
		);

		uint32_t chainIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo = {XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO, NULL};
		xrAcquireSwapchainImage(engine.ColorSwapChain, &acquireInfo, &chainIndex);
		engine.swapChainIndex = int(chainIndex);

		XrPosef xfLocalFromEye[NUM_EYES];

		// calculate eye view and projection matrices
		for (int eye = 0; eye < NUM_EYES; eye++) {
			XrPosef xfHeadFromEye = projections[eye].pose;
			xfLocalFromEye[eye] = XrPosef_Multiply(xfLocalFromHead, xfHeadFromEye);

			XrPosef xfEyeFromLocal = XrPosef_Inverse(xfLocalFromEye[eye]);

			XrMatrix4x4f viewMat = XrMatrix4x4f_CreateFromRigidTransform(&xfEyeFromLocal);

			const XrFovf fov = projections[eye].fov;
			XrMatrix4x4f projMat;
			XrMatrix4x4f_CreateProjectionFov(&projMat, GRAPHICS_OPENGL_ES, fov, 0.1f, 0.0f);

			dd_matrix_copy(&engine.View[eye], (struct dd_matrix *)viewMat.m);
			dd_matrix_copy(&engine.Proj[eye], (struct dd_matrix *)projMat.m);
		}

		for (int controllerIndex = 0; controllerIndex < 2; ++controllerIndex) {
			engine.RenderController[controllerIndex] = 1;
			XrSpace& space = controllerIndex == 0 ? engine.input.aimPoseSpaceL : engine.input.aimPoseSpaceR;
			XrSpaceLocation loc = {XR_TYPE_SPACE_LOCATION};
			xrLocateSpace(space, engine.LocalSpace, frameState.predictedDisplayTime, &loc);
			const Matrix4f transform = Matrix4f(*reinterpret_cast<OVR::Posef*>(&loc.pose));
			dd_matrix_copy(&engine.ControllerPoses[controllerIndex], (struct dd_matrix *) &transform);
		}

		XrSwapchainImageWaitInfo waitInfo;
		waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
		waitInfo.next = NULL;
		waitInfo.timeout = 1000000000; /* timeout in nanoseconds */
		XrResult res = xrWaitSwapchainImage(engine.ColorSwapChain, &waitInfo);
		int retry = 0;
		while (res == XR_TIMEOUT_EXPIRED) {
			res = xrWaitSwapchainImage(engine.ColorSwapChain, &waitInfo);
			retry++;
		}

		// draw actual project
		avdl_engine_draw(&engine);

		XrSwapchainImageReleaseInfo releaseInfo = {XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO, NULL};
		xrReleaseSwapchainImage(engine.ColorSwapChain, &releaseInfo);

		//compositor layers for this frame
		XrCompositionLayerProjectionView proj_views[2] = {};

		engine.LayerCount = 0;
		memset(engine.Layers, 0, sizeof(ovrCompositorLayer_Union) * ovrMaxLayerCount);

		XrCompositionLayerProjection proj_layer = {};
		proj_layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
		proj_layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		proj_layer.layerFlags |= XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
		proj_layer.space = engine.LocalSpace;
		proj_layer.viewCount = NUM_EYES;
		proj_layer.views = proj_views;

		for (int eye = 0; eye < NUM_EYES; eye++) {
			XrCompositionLayerProjectionView& proj_view = proj_views[eye];
			proj_view = {};
			proj_view.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
			proj_view.pose = xfLocalFromEye[eye];
			proj_view.fov = projections[eye].fov;

			proj_view.subImage.swapchain = engine.ColorSwapChain;
			proj_view.subImage.imageRect.offset.x = 0;
			proj_view.subImage.imageRect.offset.y = 0;
			proj_view.subImage.imageRect.extent.width = width;
			proj_view.subImage.imageRect.extent.height = height;
			proj_view.subImage.imageArrayIndex = eye;
		}

		engine.Layers[engine.LayerCount++].Projection = proj_layer;

		// compose the layers for this frame
		const XrCompositionLayerBaseHeader* layers[ovrMaxLayerCount] = {};
		for (int i = 0; i < engine.LayerCount; i++) {
			layers[i] = (const XrCompositionLayerBaseHeader*)&engine.Layers[i];
		}

		// finish XR frame
		XrFrameEndInfo endFrameInfo = {};
		endFrameInfo.type = XR_TYPE_FRAME_END_INFO;
		endFrameInfo.displayTime = frameState.predictedDisplayTime;
		endFrameInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		endFrameInfo.layerCount = engine.LayerCount;
		endFrameInfo.layers = layers;

		xrEndFrame(engine.session, &endFrameInfo);
	}

	avdl_engine_clean(&engine);

	free(projections);

	// egl cleanup
	if (eglDisplay != 0) {
		if (eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE) {
			dd_log("avdl error: failed to set main egl display during cleanup");
		}
	}
	if (eglContext != EGL_NO_CONTEXT) {
		if (eglDestroyContext(eglDisplay, eglContext) == EGL_FALSE) {
			dd_log("avdl error: could not destroy egl context");
		}
		eglContext = EGL_NO_CONTEXT;
	}
	if (eglTinySurface != EGL_NO_SURFACE) {
		if (eglDestroySurface(eglDisplay, eglTinySurface) == EGL_FALSE) {
			dd_log("avdl error: could not destroy egl surface");
		}
		eglTinySurface = EGL_NO_SURFACE;
	}
	if (eglDisplay != 0) {
		if (eglTerminate(eglDisplay) == EGL_FALSE) {
			dd_log("avdl error: could not terminate egl");
		}
		eglDisplay = 0;
	}

	xrDestroyInstance(instance);

	(*androidApp->activity->vm).DetachCurrentThread();

	exit(0);
}
#endif

