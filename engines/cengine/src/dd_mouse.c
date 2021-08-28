#include "dd_mouse.h"
#include "dd_game.h"

extern int input_mouse_x;
extern int input_mouse_y;
int dd_mouse_x() {
	return input_mouse_x;
}

int dd_mouse_y() {
	return input_mouse_y;
}

float dd_mouse_xProportion() {
	return (float) dd_mouse_x() / (float) dd_window_width();
}

float dd_mouse_yProportion() {
	return (float) dd_mouse_y() / (float) dd_window_height();
}
