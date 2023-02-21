#include "avdl_input.h"

void avdl_input_Init(struct AvdlInput *o) {
	o->mouse_input_total = 0;
	o->loc_x = 0;
	o->loc_y = 0;
}

int avdl_input_GetInputTotal(struct AvdlInput *o) {
	return o->mouse_input_total;
}

int avdl_input_GetButton(struct AvdlInput *o, int index) {
	if (index >= avdl_input_GetInputTotal(o)) {
		return 0;
	}
	return o->mouse_input[index].button;
}

int avdl_input_GetState(struct AvdlInput *o, int index) {
	if (index >= avdl_input_GetInputTotal(o)) {
		return 0;
	}
	return o->mouse_input[index].state;
}

int avdl_input_ClearInput(struct AvdlInput *o) {
	o->mouse_input_total = 0;
}

int avdl_input_AddInput(struct AvdlInput *o, int button, int state, int x, int y) {
	if (o->mouse_input_total >= 10) {
		return -1;
	}
	o->mouse_input[o->mouse_input_total].button = button;
	o->mouse_input[o->mouse_input_total].state = state;
	o->loc_x = x;
	o->loc_y = y;
	o->mouse_input_total++;
	return 0;
}

int avdl_input_AddPassiveMotion(struct AvdlInput *o, int x, int y) {
	o->loc_x = x;
	o->loc_y = y;
}

int avdl_input_GetX(struct AvdlInput *o) {
	return o->loc_x;
}

int avdl_input_GetY(struct AvdlInput *o) {
	return o->loc_y;
}
