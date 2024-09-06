#ifndef DD_UI_ELEMENT_H
#define DD_UI_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_mesh.h"

struct avdl_ui_element {

	// debug mesh
	struct avdl_mesh mesh_debug;

	// screen anchors
	float anchorX;
	float anchorY;

	// offset
	float offsetX;
	float offsetY;
	float offsetZ;

	// anchor parent ?
	//#(def ref Button2 anchorFrom)

	// size
	float sizeW;
	float sizeH;

	// actual
	float x;
	float y;
	float z;

	//int isDisabled;
	int isVisible;

	int isSelected;
	int isSelectedClicked;

	// onclick variable
	void (*onClick)(void *ctx);
	void (*SetOnClick)(struct avdl_ui_element *, void (*func)(), void *data);
	void *onClickData;

	// ui positional functions
	void (*SetSize)(struct avdl_ui_element *, float, float);
	void (*SetPosition)(struct avdl_ui_element *, float, float);
	void (*SetPositionZ)(struct avdl_ui_element *, float);
	void (*SetAnchor)(struct avdl_ui_element *, float, float);

	void (*create)(struct avdl_ui_element *);
	void (*update)(struct avdl_ui_element *, float dt);
	void (*applyTransform)(struct avdl_ui_element *);
	void (*drawDebug)(struct avdl_ui_element *);
	void (*clean)(struct avdl_ui_element *);
	void (*resize)(struct avdl_ui_element *);

	void (*mouse_input)(struct avdl_ui_element *, int, int);
	int (*hasMouseCollided)(struct avdl_ui_element *);

	void (*disable)(struct avdl_ui_element *);

	int (*IsSelected)(struct avdl_ui_element *);
	int (*IsClicked)(struct avdl_ui_element *);

	float (*GetPositionX)(struct avdl_ui_element *);
	float (*GetPositionY)(struct avdl_ui_element *);
	float (*GetPositionZ)(struct avdl_ui_element *);

	int (*IsVisible)(struct avdl_ui_element *);
	void (*SetVisible)(struct avdl_ui_element *, int value);

};

// ui positional functions
void avdl_ui_element_SetSize(struct avdl_ui_element *, float, float);
void avdl_ui_element_SetPosition(struct avdl_ui_element *, float, float);
void avdl_ui_element_SetPositionZ(struct avdl_ui_element *, float);
void avdl_ui_element_SetAnchor(struct avdl_ui_element *, float, float);

void avdl_ui_element_create(struct avdl_ui_element *);
void avdl_ui_element_update(struct avdl_ui_element *, float dt);
void avdl_ui_element_applyTransform(struct avdl_ui_element *);
void avdl_ui_element_drawDebug(struct avdl_ui_element *);
void avdl_ui_element_clean(struct avdl_ui_element *);
void avdl_ui_element_resize(struct avdl_ui_element *);

void avdl_ui_element_mouse_input(struct avdl_ui_element *, int, int);
int avdl_ui_element_hasMouseCollided(struct avdl_ui_element *);

void avdl_ui_element_disable(struct avdl_ui_element *);
void avdl_ui_element_SetOnClick(struct avdl_ui_element *, void (*func)(), void *data);

int avdl_ui_element_IsSelected(struct avdl_ui_element *);
int avdl_ui_element_IsClicked(struct avdl_ui_element *);

float avdl_ui_element_GetPositionX(struct avdl_ui_element *);
float avdl_ui_element_GetPositionY(struct avdl_ui_element *);
float avdl_ui_element_GetPositionZ(struct avdl_ui_element *);

int avdl_ui_element_IsVisible(struct avdl_ui_element *);
void avdl_ui_element_SetVisible(struct avdl_ui_element *, int value);

#ifdef __cplusplus
}
#endif

#endif
