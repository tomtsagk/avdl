#include "avdl_input.h"
#include <string.h>
#include "dd_log.h"
#include "avdl_engine.h"

extern struct avdl_engine engine;

#if defined(AVDL_QUEST2)
XrAction create_action(XrActionSet actionSet, XrActionType type, const char *name, const char *locName) {
	XrActionCreateInfo aci = {XR_TYPE_ACTION_CREATE_INFO};
	aci.actionType = type;
	strcpy(aci.actionName, name);
	strcpy(aci.localizedActionName, locName);
	//aci.countSubactionPaths = subcount;
	//aci.subactionPaths = subpaths;
	XrAction action = XR_NULL_HANDLE;
	xrCreateAction(actionSet, &aci, &action);
	return action;
}

void set_binding(XrActionSuggestedBinding *b, XrInstance instance, XrAction action, const char *path) {
	b->action = action;
	XrPath bindingPath;
	xrStringToPath(instance, path, &bindingPath);
	b->binding = bindingPath;
}

void get_state(XrActionStateBoolean *state, XrSession session, XrAction action) {
        XrActionStateGetInfo getInfo = {XR_TYPE_ACTION_STATE_GET_INFO};
        getInfo.action = action;
        getInfo.subactionPath = XR_NULL_PATH;
        xrGetActionStateBoolean(session, &getInfo, state);
}

#endif

void avdl_input_Init(struct AvdlInput *o) {
	o->input_total = 0;
	o->loc_x = 0;
	o->loc_y = 0;

#if defined(AVDL_QUEST2)
	o->actionSet = XR_NULL_HANDLE;
	o->a = XR_NULL_HANDLE;
	o->b = XR_NULL_HANDLE;
	o->x = XR_NULL_HANDLE;
	o->y = XR_NULL_HANDLE;
	o->triggerL = XR_NULL_HANDLE;
	o->triggerR = XR_NULL_HANDLE;
	o->gripL = XR_NULL_HANDLE;
	o->gripR = XR_NULL_HANDLE;
	o->gripPoseL = XR_NULL_HANDLE;
	o->gripPoseR = XR_NULL_HANDLE;
	o->aimPoseL = XR_NULL_HANDLE;
	o->aimPoseR = XR_NULL_HANDLE;
	o->menu = XR_NULL_HANDLE;

	// oculus touch controllers profile
        XrPath interactionProfile;
        xrStringToPath(o->instance, "/interaction_profiles/oculus/touch_controller", &interactionProfile);

	// Create an action set
	XrActionSetCreateInfo asci = {XR_TYPE_ACTION_SET_CREATE_INFO};
	asci.priority = 0;
	strcpy(asci.actionSetName, "avdl_actions");
	strcpy(asci.localizedActionSetName, "Avdl Actions");
	xrCreateActionSet(o->instance, &asci, &o->actionSet);

	// actions
	o->a = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "a", "A Button");
	o->b = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "b", "B Button");
	o->x = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "x", "X Button");
	o->y = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "y", "Y Button");
	o->triggerL = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "trigger_l", "Left Trigger");
	o->triggerR = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "trigger_r", "Right Trigger");
	o->gripL = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "grip_l", "Left Grip");
	o->gripR = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "grip_r", "Right Grip");
	o->menu = create_action(o->actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "menu", "Menu");
	o->aimPoseL = create_action(o->actionSet, XR_ACTION_TYPE_POSE_INPUT, "aim_pose_l", "Left Aim Pose");
	o->aimPoseR = create_action(o->actionSet, XR_ACTION_TYPE_POSE_INPUT, "aim_pose_r", "Right Aim Pose");
	o->gripPoseL = create_action(o->actionSet, XR_ACTION_TYPE_POSE_INPUT, "grip_pose_l", "Left Grip Pose");
	o->gripPoseR = create_action(o->actionSet, XR_ACTION_TYPE_POSE_INPUT, "grip_pose_r", "Right Grip Pose");

	// combine action and binding
	const int bindings_total = 13;
	XrActionSuggestedBinding bindings[bindings_total];
	set_binding(&bindings[0], o->instance, o->a, "/user/hand/right/input/a/click");
	set_binding(&bindings[1], o->instance, o->b, "/user/hand/right/input/b/click");
	set_binding(&bindings[2], o->instance, o->x, "/user/hand/left/input/x/click");
	set_binding(&bindings[3], o->instance, o->y, "/user/hand/left/input/y/click");
	set_binding(&bindings[4], o->instance, o->triggerL, "/user/hand/left/input/trigger");
	set_binding(&bindings[5], o->instance, o->triggerR, "/user/hand/right/input/trigger");
	set_binding(&bindings[6], o->instance, o->gripL, "/user/hand/left/input/squeeze");
	set_binding(&bindings[7], o->instance, o->gripR, "/user/hand/right/input/squeeze");
	set_binding(&bindings[8], o->instance, o->menu, "/user/hand/left/input/menu/click");
	set_binding(&bindings[9], o->instance, o->aimPoseL, "/user/hand/left/input/aim/pose");
	set_binding(&bindings[10], o->instance, o->aimPoseR, "/user/hand/right/input/aim/pose");
	set_binding(&bindings[11], o->instance, o->gripPoseL, "/user/hand/left/input/grip/pose");
	set_binding(&bindings[12], o->instance, o->gripPoseR, "/user/hand/right/input/grip/pose");

	/*
	 * unused
	 details in OpenXR/Include/openxr/fb_touch_controller_pro.h
	/user/hand/<both>/input/trigger/value (float)
	/user/hand/<both>/input/squeeze/value (float)
	/user/hand/<both>/input/squeeze/touch (bool?)
	//    …/input/thumbstick
	//    …/input/thumbstick/x
	//    …/input/thumbstick/y
	//    …/input/thumbstick/click
	//    …/input/thumbstick/touch
	//    …/input/thumbrest/touch
	//    …/input/grip/pose
	//    …/input/aim/pose
	//    …/output/haptic

	???
	//    …/input/thumbrest/force
	//    …/input/stylus_fb/force
	//    …/input/trigger/curl_fb
	//    …/input/trigger/slide_fb
	//    …/input/trigger/proximity_fb
	//    …/input/thumb_fb/proximity_fb
	//    …/output/trigger_haptic_fb
	//    …/output/thumb_haptic_fb
	 */

	// Suggest bindings
	XrInteractionProfileSuggestedBinding suggestedBindings = {XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
	suggestedBindings.interactionProfile = interactionProfile;
	suggestedBindings.suggestedBindings = &bindings[0];
	suggestedBindings.countSuggestedBindings = bindings_total;
	xrSuggestInteractionProfileBindings(o->instance, &suggestedBindings);

	// attach action set to session
	XrSessionActionSetsAttachInfo attachInfo = {XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
	attachInfo.countActionSets = 1;
	attachInfo.actionSets = &o->actionSet;
	xrAttachSessionActionSets(o->session, &attachInfo);

	XrActionSpaceCreateInfo asciL = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
	asciL.action = o->gripPoseL;
	asciL.poseInActionSpace.orientation.w = 1.0f;
	asciL.subactionPath = XR_NULL_PATH;
	XrSpace actionSpaceL = XR_NULL_HANDLE;
	xrCreateActionSpace(o->session, &asciL, &actionSpaceL);
	o->gripPoseSpaceL = actionSpaceL;

	XrActionSpaceCreateInfo asciR = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
	asciR.action = o->gripPoseR;
	asciR.poseInActionSpace.orientation.w = 1.0f;
	asciR.subactionPath = XR_NULL_PATH;
	XrSpace actionSpaceR = XR_NULL_HANDLE;
	xrCreateActionSpace(o->session, &asciR, &actionSpaceR);
	o->gripPoseSpaceR = actionSpaceR;

	XrActionSpaceCreateInfo asciL2 = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
	asciL2.action = o->aimPoseL;
	asciL2.poseInActionSpace.orientation.w = 1.0f;
	asciL2.subactionPath = XR_NULL_PATH;
	XrSpace actionSpaceL2 = XR_NULL_HANDLE;
	xrCreateActionSpace(o->session, &asciL2, &actionSpaceL2);
	o->aimPoseSpaceL = actionSpaceL2;

	XrActionSpaceCreateInfo asciR2 = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
	asciR2.action = o->aimPoseR;
	asciR2.poseInActionSpace.orientation.w = 1.0f;
	asciR2.subactionPath = XR_NULL_PATH;
	XrSpace actionSpaceR2 = XR_NULL_HANDLE;
	xrCreateActionSpace(o->session, &asciR2, &actionSpaceR2);
	o->aimPoseSpaceR = actionSpaceR2;

#endif

}

void avdl_input_update(struct AvdlInput *o) {

#if defined(AVDL_QUEST2)
	// sync action data
	XrActiveActionSet activeActionSet = {};
	activeActionSet.actionSet = o->actionSet;
	activeActionSet.subactionPath = XR_NULL_PATH;

	XrActionsSyncInfo syncInfo = {XR_TYPE_ACTIONS_SYNC_INFO};
	syncInfo.countActiveActionSets = 1;
	syncInfo.activeActionSets = &activeActionSet;
	xrSyncActions(o->session, &syncInfo);

        XrActionStateBoolean state = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&state, o->session, o->a);

        XrActionStateBoolean stateB = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateB, o->session, o->b);

        XrActionStateBoolean stateX = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateX, o->session, o->x);

        XrActionStateBoolean stateY = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateY, o->session, o->y);

        XrActionStateBoolean stateTriggerL = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateTriggerL, o->session, o->triggerL);

        XrActionStateBoolean stateTriggerR = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateTriggerR, o->session, o->triggerR);

        XrActionStateBoolean stateGripL = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateGripL, o->session, o->gripL);

        XrActionStateBoolean stateGripR = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateGripR, o->session, o->gripR);

        XrActionStateBoolean stateMenu = {XR_TYPE_ACTION_STATE_BOOLEAN};
	get_state(&stateMenu, o->session, o->menu);

/*
        if (state.changedSinceLastSync == XR_TRUE && state.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_A;
	}

        if (stateB.changedSinceLastSync == XR_TRUE && stateB.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_B;
	}

        if (stateX.changedSinceLastSync == XR_TRUE && stateX.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_X;
	}

        if (stateY.changedSinceLastSync == XR_TRUE && stateY.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_Y;
	}

        if (stateTriggerL.changedSinceLastSync == XR_TRUE && stateTriggerL.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_TRIGGER_L;
	}

        if (stateTriggerR.changedSinceLastSync == XR_TRUE && stateTriggerR.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_TRIGGER_R;
	}

        if (stateGripL.changedSinceLastSync == XR_TRUE && stateGripL.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_GRIP_L;
	}

        if (stateGripR.changedSinceLastSync == XR_TRUE && stateGripR.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_GRIP_R;
	}

        if (stateMenu.changedSinceLastSync == XR_TRUE && stateMenu.currentState == XR_TRUE) {
		o->input_key = AVDL_INPUT_QUEST2_MENU;
	}
	*/

	#endif

}

int avdl_input_GetInputTotal(struct AvdlInput *o) {
	return o->input_total;
}

int avdl_input_GetButton(struct AvdlInput *o, int index) {
	if (index >= avdl_input_GetInputTotal(o)) {
		return 0;
	}
	return o->input[index].button;
}

int avdl_input_GetState(struct AvdlInput *o, int index) {
	if (index >= avdl_input_GetInputTotal(o)) {
		return 0;
	}
	return o->input[index].state;
}

int avdl_input_ClearInput(struct AvdlInput *o) {
	o->input_total = 0;
	return 0;
}

int avdl_input_AddInput(struct AvdlInput *o, int button, int state) {
	if (o->input_total >= AVDL_INPUT_KEYS_MAXIMUM) {
		return -1;
	}
	o->input[o->input_total].button = button;
	o->input[o->input_total].state = state;
	o->input_total++;
	return 0;
}

int avdl_input_AddInputLocation(struct AvdlInput *o, int button, int state, int x, int y) {
	if (o->input_total >=AVDL_INPUT_KEYS_MAXIMUM) {
		return -1;
	}
	o->input[o->input_total].button = button;
	o->input[o->input_total].state = state;
	o->loc_x = x;
	o->loc_y = y;
	o->input_total++;
	return 0;
}

int avdl_input_AddPassiveMotion(struct AvdlInput *o, int x, int y) {
	o->loc_x = x;
	o->loc_y = y;
	return 0;
}

int avdl_input_GetX(struct AvdlInput *o) {
	return o->loc_x;
}

int avdl_input_GetY(struct AvdlInput *o) {
	return o->loc_y;
}
