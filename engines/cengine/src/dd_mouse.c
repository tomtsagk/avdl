#include "dd_mouse.h"
#include "dd_game.h"
#include "avdl_input.h"

extern struct AvdlInput avdl_input;
int dd_mouse_x() {
	return avdl_input_GetX(&avdl_input);
}

int dd_mouse_y() {
	return avdl_input_GetY(&avdl_input);
}

float dd_mouse_xProportion() {
	return (float) dd_mouse_x() / (float) dd_window_width();
}

float dd_mouse_yProportion() {
	return (float) dd_mouse_y() / (float) dd_window_height();
}
