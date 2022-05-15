#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_cengine.h"

static struct dd_image fontTexture;
extern GLuint fontProgram;
extern GLuint defaultProgram;
extern GLuint currentProgram;

static int isActive = 0;
static const char *fontname = 0;

static int fontColumns = 1;
static int fontRows = 1;
static float fontWidth = 1.0;
static float fontHeight = 1.0;

void dd_string3d_activate(const char *src, float fColumns, float fRows, float fWidth, float fHeight) {
	isActive = 1;
	fontname = src;

	fontColumns = fColumns;
	fontRows = fRows;
	fontWidth = fWidth;
	fontHeight = fHeight;

}

int dd_string3d_isActive() {
	return isActive;
}

extern struct dd_matrix matPerspective;

void dd_string3d_init() {

	dd_image_create(&fontTexture);
	dd_image_set(&fontTexture, fontname);

} // string3d init

void dd_string3d_create(struct dd_string3d *o) {

	dd_da_init(&o->textMeshes, sizeof(struct dd_meshTexture));

	o->align = DD_STRING3D_ALIGN_LEFT;
	o->colorFront[0] = 1.0;
	o->colorFront[1] = 1.0;
	o->colorFront[2] = 1.0;
	o->colorBack[0] = 0.0;
	o->colorBack[1] = 0.0;
	o->colorBack[2] = 0.0;
	o->len = 0;

	o->setAlign = dd_string3d_setAlign;
	o->clean = dd_string3d_clean;
	o->draw = dd_string3d_draw;
	o->drawInt = dd_string3d_drawInt;
	o->drawLimit = dd_string3d_drawLimit;
	o->setText = dd_string3d_setText;

}

void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al) {
	o->align = al;
}

void dd_string3d_draw(struct dd_string3d *o) {
	dd_string3d_drawLimit(o, 0);
}

void dd_string3d_drawInt(struct dd_string3d *o, int num) {
	/*
	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	dd_string3d_draw(o, numberString);
	*/
}

void dd_string3d_drawLimit(struct dd_string3d *o, int limit) {

	if (o->textMeshes.elements > 0) {
		struct dd_meshTexture *m = dd_da_get(&o->textMeshes, 0);

		dd_matrix_push();

		switch (o->align) {
		case DD_STRING3D_ALIGN_LEFT:
			break;
		case DD_STRING3D_ALIGN_CENTER:
			dd_translatef(0.5 -(o->len *0.5), 0, 0);
			break;
		case DD_STRING3D_ALIGN_RIGHT:
			dd_translatef(0.5 -(o->len), 0, 0);
			break;
		}

		int previousProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
		glUseProgram(fontProgram);
		GLint MatrixID = glGetUniformLocation(fontProgram, "matrix");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

		dd_meshTexture_draw(m);

		glUseProgram(previousProgram);
		dd_matrix_pop();
	}
}

void dd_string3d_clean(struct dd_string3d *o) {
	dd_da_free(&o->textMeshes);
}

void dd_string3d_setText(struct dd_string3d *o, const char *text) {

	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_meshTexture *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(p);
	}
	dd_da_empty(&o->textMeshes);

	// add new text meshes
	struct dd_meshTexture m;
	dd_da_add(&o->textMeshes, &m);

	struct dd_meshTexture *p;
	p = dd_da_get(&o->textMeshes, 0);

	dd_meshTexture_create(p);
	dd_meshTexture_setTexture(p, &fontTexture);

	o->len = strlen(text);

	int len = strlen(text);
	for (int i = 0; i < len; i++) {

		struct dd_meshTexture m2;
		dd_meshTexture_create(&m2);
		dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);
		dd_meshColour_set_colour(&m2, 1, 1, 1);

		// for each letter, create a mesh and position it
		int offsetX = (text[i] -32) %14;
		int offsetY = 6 -((text[i] -32) /14);
		dd_meshTexture_set_primitive_texcoords(&m2,
			((fontWidth /fontColumns) *offsetX),
			(1.0 -fontHeight) +((fontHeight /fontRows) *offsetY),
			(fontWidth /fontColumns),
			(fontHeight /fontRows)
		);

		dd_meshTexture_combine(p, &m2, i *1, 0, 0);
	}

}
