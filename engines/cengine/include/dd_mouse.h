#ifndef DD_MOUSE_H
#define DD_MOUSE_H

enum DD_INPUT_MOUSE_BUTTON {
	DD_INPUT_MOUSE_BUTTON_LEFT,
	DD_INPUT_MOUSE_BUTTON_MIDDLE,
	DD_INPUT_MOUSE_BUTTON_RIGHT,
};

enum DD_INPUT_MOUSE_TYPE {
	DD_INPUT_MOUSE_TYPE_PRESSED,
	DD_INPUT_MOUSE_TYPE_MOVE,
	DD_INPUT_MOUSE_TYPE_RELEASED,
};

/* mouse
 *
 * `dd_mouse_?` functions return the exact pixel the mouse is on
 * `dd_mouse_?Proportion` functions return the position
 * 	of the mouse, compared to the window width
 * 	and ranges from 0 to 1.
 */
int dd_mouse_x();
int dd_mouse_y();
float dd_mouse_xProportion();
float dd_mouse_yProportion();

#if DD_PLATFORM_ANDROID
#define DD_MOUSE_SHAPE_INHERIT 0
#define DD_MOUSE_SHAPE_NONE 0
#define dd_mouse_shape(shape)
#define dd_mouse_position(x, y)
#elif DD_PLATFORM_NATIVE
#define DD_MOUSE_SHAPE_INHERIT GLUT_CURSOR_INHERIT
#define DD_MOUSE_SHAPE_NONE GLUT_CURSOR_NONE
// temporarily disable
//#define dd_mouse_shape(shape) glutSetCursor(shape)
//#define dd_mouse_position(x, y) glutWarpPointer(x, y)
#endif

#endif
