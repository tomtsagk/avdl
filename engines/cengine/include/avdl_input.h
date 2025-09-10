#ifndef AVDL_INPUT_H
#define AVDL_INPUT_H

#include "avdl_graphics.h"
#include "shared/avdl_string.h"

#if defined(AVDL_QUEST2)
#include <jni.h>
#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1
#include <openxr/openxr.h>
#include <openxr/openxr_oculus.h>
#include <openxr/openxr_oculus_helpers.h>
#include <openxr/openxr_platform.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum AVDL_INPUT_STATES {
	AVDL_INPUT_STATE_DOWN,
	AVDL_INPUT_STATE_UP,
	AVDL_INPUT_STATE_MOVE, // mouse only
};

// Custom input keys, must be above ASCII
enum AVDL_INPUT_KEYS {
	AVDL_INPUT_QUEST2_A = 300,
	AVDL_INPUT_QUEST2_B,
	AVDL_INPUT_QUEST2_X,
	AVDL_INPUT_QUEST2_Y,
	AVDL_INPUT_QUEST2_TRIGGER_L,
	AVDL_INPUT_QUEST2_TRIGGER_R,
	AVDL_INPUT_QUEST2_GRIP_L,
	AVDL_INPUT_QUEST2_GRIP_R,
	AVDL_INPUT_QUEST2_MENU,
	AVDL_INPUT_ARROW_UP = 400,
	AVDL_INPUT_ARROW_RIGHT,
	AVDL_INPUT_ARROW_DOWN,
	AVDL_INPUT_ARROW_LEFT,
	AVDL_INPUT_MOUSE_LEFT,
	AVDL_INPUT_MOUSE_MIDDLE,
	AVDL_INPUT_MOUSE_RIGHT,
	AVDL_INPUT_DROPFILE,
};

#define AVDL_INPUT_KEYS_MAXIMUM 100

// avdl input

struct avdl_input {
	int button;
	int state;
	struct avdl_string filename;
};
int avdl_input_GetButton(struct avdl_input *);
int avdl_input_GetState(struct avdl_input *);
struct avdl_string *avdl_input_GetFilename(struct avdl_input *);

// avdl input manager

struct avdl_inputmanager {

	// input keys
	struct avdl_input input[AVDL_INPUT_KEYS_MAXIMUM];
	int input_total;
	int loc_x;
	int loc_y;

	#if defined(AVDL_QUEST2)
	XrActionSet actionSet;
	XrAction a;
	XrAction b;
	XrAction x;
	XrAction y;
	XrAction triggerL;
	XrAction triggerR;
	XrAction gripL;
	XrAction gripR;
	XrAction gripPoseL;
	XrAction gripPoseR;
	XrAction aimPoseL;
	XrAction aimPoseR;
	XrAction menu;
	XrSpace gripPoseSpaceL;
	XrSpace gripPoseSpaceR;
	XrSpace aimPoseSpaceL;
	XrSpace aimPoseSpaceR;
	XrInstance instance;
	XrSession session;
	XrPath leftHandPath;
	XrPath rightHandPath;
	#endif
};

void avdl_inputmanager_Init(struct avdl_inputmanager *);
void avdl_inputmanager_update(struct avdl_inputmanager *);
int avdl_inputmanager_GetInputTotal(struct avdl_inputmanager *);
int avdl_inputmanager_GetX(struct avdl_inputmanager *);
int avdl_inputmanager_GetY(struct avdl_inputmanager *);
int avdl_inputmanager_ClearInput(struct avdl_inputmanager *);

int avdl_inputmanager_AddInputDropfile(struct avdl_inputmanager *, const char *filename);
int avdl_inputmanager_AddInput(struct avdl_inputmanager *, int button, int state);
int avdl_inputmanager_AddInputLocation(struct avdl_inputmanager *, int button, int state, int x, int y);
int avdl_inputmanager_AddPassiveMotion(struct avdl_inputmanager *, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
