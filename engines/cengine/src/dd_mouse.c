#include "dd_mouse.h"
#include "dd_game.h"
#include "avdl_screen.h"
#include "avdl_input.h"
#include "avdl_engine.h"

extern struct avdl_engine engine;
int dd_mouse_x() {
	return avdl_inputmanager_GetX(&engine.input);
}

int dd_mouse_y() {
	return avdl_inputmanager_GetY(&engine.input);
}

float dd_mouse_xProportion() {
	return (float) dd_mouse_x() / (float) avdl_screen_GetWidth();
}

float dd_mouse_yProportion() {
	return (float) dd_mouse_y() / (float) avdl_screen_GetHeight();
}
