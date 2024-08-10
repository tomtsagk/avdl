#include "avdl_ui_element.h"
#include "dd_matrix.h"
#include "dd_log.h"
#include "dd_mouse.h"
#include "dd_game.h"

void avdl_ui_element_create(struct avdl_ui_element *o) {
	o->SetSize = avdl_ui_element_SetSize;
	o->SetPosition = avdl_ui_element_SetPosition;
	o->SetPositionZ = avdl_ui_element_SetPositionZ;
	o->SetAnchor = avdl_ui_element_SetAnchor;

	o->create = avdl_ui_element_create;
	o->update = avdl_ui_element_update;
	o->applyTransform = avdl_ui_element_applyTransform;
	o->drawDebug = avdl_ui_element_drawDebug;
	o->clean = avdl_ui_element_clean;
	o->resize = avdl_ui_element_resize;

	o->mouse_input = avdl_ui_element_mouse_input;
	o->hasMouseCollided = avdl_ui_element_hasMouseCollided;

	o->disable = avdl_ui_element_disable;

	o->IsSelected = avdl_ui_element_IsSelected;
	o->IsClicked = avdl_ui_element_IsClicked;

	o->SetOnClick = avdl_ui_element_SetOnClick;
	o->onClick = 0;
	o->onClickData = 0;

	o->GetPositionX = avdl_ui_element_GetPositionX;
	o->GetPositionY = avdl_ui_element_GetPositionY;
	o->GetPositionZ = avdl_ui_element_GetPositionZ;

	o->IsVisible = avdl_ui_element_IsVisible;
	o->SetVisible = avdl_ui_element_SetVisible;

	// screen anchors
	o->anchorX = 0.5;
	o->anchorY = 0.5;

	// offset
	o->offsetX = 0;
	o->offsetY = 0;
	o->offsetZ = 0;

	// anchor parent ?
	//#(def ref Button2 anchorFrom)

	// size
	o->sizeW = 1;
	o->sizeH = 1;

	// actual
	o->x = 0;
	o->y = 0;

	//int isDisabled;
	o->isVisible = 1;

	o->isSelected = 0;
	o->isSelectedClicked = 0;

	avdl_mesh_create(&o->mesh_debug);
	o->mesh_debug.set_primitive(&o->mesh_debug, AVDL_PRIMITIVE_BOX);
	o->mesh_debug.setWireframe(&o->mesh_debug);
}

void avdl_ui_element_SetSize(struct avdl_ui_element *o, float width, float height) {
	o->sizeW = width;
	o->sizeH = height;
	avdl_ui_element_resize(o);
}

void avdl_ui_element_SetPosition(struct avdl_ui_element *o, float x, float y) {
	o->offsetX = x;
	o->offsetY = y;
	avdl_ui_element_resize(o);
}

void avdl_ui_element_SetPositionZ(struct avdl_ui_element *o, float z) {
	o->offsetZ = z;
	avdl_ui_element_resize(o);
}

void avdl_ui_element_update(struct avdl_ui_element *o, float dt) {
	if (o->isVisible == 0) {
		return;
	}

	if (o->hasMouseCollided(o)) {
		o->isSelected = 1;
	}
	else {
		o->isSelected = 0;
	}
}

void avdl_ui_element_applyTransform(struct avdl_ui_element *o) {
	dd_translatef(o->x, o->y, -5.0 +o->offsetZ);
}

void avdl_ui_element_drawDebug(struct avdl_ui_element *o) {

	if (o->isVisible == 0) {
		return;
	}

	// debug
	dd_pushMatrix();
	o->applyTransform(o);
	dd_scalef(o->sizeW, o->sizeH, 0.01);
	if (o->isSelected) {
		dd_scalef(1.2, 1.2, 1.2);
	}
	if (o->isSelectedClicked) {
		dd_scalef(1.2, 1.2, 1.2);
	}
	o->mesh_debug.draw(&o->mesh_debug);
	dd_popMatrix();

}

void avdl_ui_element_clean(struct avdl_ui_element *o) {
	avdl_mesh_clean(&o->mesh_debug);
}

int avdl_ui_element_hasMouseCollided(struct avdl_ui_element *o) {

	if (o->isVisible == 0) {
		return 0;
	}

	// get mouse's proportioned position
	float screenProportionX;
	float screenProportionY;
	screenProportionX = dd_mouse_xProportion() -0.5;
	screenProportionY = dd_mouse_yProportion() -0.5;

	// get x and y of the plane where the button is
	float planeX;
	float planeY;
	planeX = screenProportionX *dd_screen_width_get ( 5);
	planeY = screenProportionY *dd_screen_height_get(-5);

	//dd_log("Plane: %f %f - %f %f - %f %f", screenProportionX, screenProportionY, dd_screen_width_get(-5), dd_screen_height_get(-5), o->x, o->y);

	// check collision
	if (planeX >= (o->x -(o->sizeW /2))
	&&  planeX <= (o->x +(o->sizeW /2))
	&&  planeY >= (o->y -(o->sizeH /2))
	&&  planeY <= (o->y +(o->sizeH /2))
	) {
		return 1;
	}

	return 0;
}

void avdl_ui_element_resize(struct avdl_ui_element *o) {
	o->x = dd_screen_width_get (5) *(o->anchorX -0.5) * 1 +o->offsetX;
	o->y = dd_screen_height_get(5) *(o->anchorY -0.5) *-1 +o->offsetY;
}

void avdl_ui_element_disable(struct avdl_ui_element *o) {
	//(= this.isDisabled 1)
}

void avdl_ui_element_SetAnchor(struct avdl_ui_element *o, float x, float y) {
	o->anchorX = x;
	o->anchorY = y;
	o->resize(o);
}

void avdl_ui_element_mouse_input(struct avdl_ui_element *o, int button, int type) {

	// set click state on selected button
	if (type == DD_INPUT_MOUSE_TYPE_PRESSED) {
		if (o->isSelected && o->hasMouseCollided(o)) {
			o->isSelectedClicked = 1;
		}
	}
	else
	// apply selected button if it was clicked
	if (type == DD_INPUT_MOUSE_TYPE_RELEASED) {
	
		if (o->isSelectedClicked && o->hasMouseCollided(o)) {
			// onclick
			if (o->onClick) {
				o->onClick(o->onClickData);
			}
		}
		o->isSelectedClicked = 0;
	}

}

void avdl_ui_element_SetOnClick(struct avdl_ui_element *o, void (*func)(), void *data) {
	o->onClick = func;
	o->onClickData = data;
}

int avdl_ui_element_IsSelected(struct avdl_ui_element *o) {
	return o->isSelected;
}

int avdl_ui_element_IsClicked(struct avdl_ui_element *o) {
	return o->isSelectedClicked;
}

float avdl_ui_element_GetPositionX(struct avdl_ui_element *o) {
	return o->offsetX;
}

float avdl_ui_element_GetPositionY(struct avdl_ui_element *o) {
	return o->offsetY;
}

float avdl_ui_element_GetPositionZ(struct avdl_ui_element *o) {
	return o->offsetZ;
}

int avdl_ui_element_IsVisible(struct avdl_ui_element *o) {
	return o->isVisible;
}

void avdl_ui_element_SetVisible(struct avdl_ui_element *o, int value) {
	o->isVisible = value;
}
