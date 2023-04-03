#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_cengine.h"

static struct dd_image fontTexture;
#ifndef AVDL_DIRECT3D11
extern GLuint fontProgram;
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

static int isActive = 0;
static const char *fontname = 0;
static int fonttype = 0;

static int fontColumns = 1;
static int fontRows = 1;
static float fontWidth = 1.0;
static float fontHeight = 1.0;

static float fontKerning = 1.0;

struct dd_meshTexture numbers[10];

void dd_string3d_activate(const char *src, int src_type, float fColumns, float fRows, float fWidth, float fHeight) {
	isActive = 1;
	fontname = src;
	fonttype = src_type;

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

	#ifndef AVDL_DIRECT3D11
	dd_image_create(&fontTexture);
	dd_image_set(&fontTexture, fontname, fonttype);

	for (int i = 0; i < 10; i++) {
		dd_meshTexture_create(&numbers[i]);
		dd_meshTexture_set_primitive(&numbers[i], DD_PRIMITIVE_RECTANGLE);
		dd_meshColour_set_colour(&numbers[i], 1, 1, 1);
		dd_meshTexture_setTexture(&numbers[i], &fontTexture);

		// for each letter, create a mesh and position it
		int offsetX = (16 +i) %fontColumns;
		int offsetY = (fontColumns-1) -((16 +i) /fontColumns);
		dd_meshTexture_set_primitive_texcoords(&numbers[i],
			((fontWidth /fontColumns) *offsetX),
			(1.0 -fontHeight) +((fontHeight /fontRows) *offsetY),
			(fontWidth /fontColumns),
			(fontHeight /fontRows)
		);
	}

	#endif

} // string3d init

void dd_string3d_create(struct dd_string3d *o) {

	dd_da_init(&o->textMeshes, sizeof(struct dd_word_mesh));

	o->align = DD_STRING3D_ALIGN_LEFT;
	o->colorFront[0] = 1.0;
	o->colorFront[1] = 1.0;
	o->colorFront[2] = 1.0;
	o->colorBack[0] = 0.0;
	o->colorBack[1] = 0.0;
	o->colorBack[2] = 0.0;

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
	#ifndef AVDL_DIRECT3D11
	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	numberString[10] = '\0';
	dd_matrix_push();

	int lineWidth = 0;

	lineWidth = strlen(numberString);

	dd_translatef(fontKerning*0.5, 0, 0);

	switch (o->align) {
	case DD_STRING3D_ALIGN_LEFT:
		break;
	case DD_STRING3D_ALIGN_CENTER:
		dd_translatef(-lineWidth *0.5 *fontKerning, 0, 0);
		break;
	case DD_STRING3D_ALIGN_RIGHT:
		dd_translatef(-lineWidth *fontKerning, 0, 0);
		break;
	}

	for (int i = 0; i < lineWidth; i++) {
		struct dd_meshTexture *m = &numbers[numberString[i] -48];

		int previousProgram;
		previousProgram = avdl_graphics_GetCurrentProgram();
		avdl_graphics_UseProgram(fontProgram);
		GLint MatrixID = avdl_graphics_GetUniformLocation(fontProgram, "matrix");
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

		dd_meshTexture_draw(m);

		avdl_graphics_UseProgram(previousProgram);

		dd_translatef(fontKerning, 0, 0);
	}

	dd_matrix_pop();
	#endif
}

void dd_string3d_drawLimit(struct dd_string3d *o, int limit) {
	#ifndef AVDL_DIRECT3D11

	dd_matrix_push();

	int wordsTotal = 0;

	// for each line
	do {
		int lineWords = 0;
		int lineWidth = 0;

		for (int i = wordsTotal; i < o->textMeshes.elements; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, i);

			// fits in the same line
			if (!limit
			|| (!lineWords && m->width > limit)
			|| lineWidth +m->width <= limit) {
				// not first word, add space
				if (lineWords != 0) {
					lineWidth++;
				}
				lineWidth += m->width;
				lineWords++;
			}
			// doesn't fit in line
			else {
				break;
			}
		}

		dd_matrix_push();
		switch (o->align) {
		case DD_STRING3D_ALIGN_LEFT:
			break;
		case DD_STRING3D_ALIGN_CENTER:
			dd_translatef(-lineWidth *0.5 *fontKerning, 0, 0);
			break;
		case DD_STRING3D_ALIGN_RIGHT:
			dd_translatef(-lineWidth *fontKerning, 0, 0);
			break;
		}

		//for (int i = 0; i < o->textMeshes.elements; i++) {
		for (int i = 0; i < lineWords; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, wordsTotal +i);

			int previousProgram;
			previousProgram = avdl_graphics_GetCurrentProgram();
			avdl_graphics_UseProgram(fontProgram);
			GLint MatrixID = avdl_graphics_GetUniformLocation(fontProgram, "matrix");
			avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

			dd_meshTexture_draw(&m->m);

			avdl_graphics_UseProgram(previousProgram);

			dd_translatef((m->width+1) *fontKerning, 0, 0);
		}
		wordsTotal += lineWords;
		dd_matrix_pop();
		dd_translatef(0, -1, 0);
	} while (wordsTotal < o->textMeshes.elements);

	dd_matrix_pop();
	#endif
}

void dd_string3d_clean(struct dd_string3d *o) {
	dd_da_free(&o->textMeshes);
}

void dd_string3d_setText(struct dd_string3d *o, const char *text) {

	#ifndef AVDL_DIRECT3D11
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
	}
	dd_da_empty(&o->textMeshes);

	// add new text meshes
	struct dd_word_mesh m;
	struct dd_word_mesh *p;

	char *t = text;

	do {
		// skip whitespace
		if (t[0] == ' ') {
			t++;
			continue;
		}

		// create new mesh for the new word
		dd_da_add(&o->textMeshes, &m);
		p = dd_da_get(&o->textMeshes, o->textMeshes.elements-1);

		dd_meshTexture_create(&p->m);
		dd_meshTexture_setTexture(&p->m, &fontTexture);

		// find characters until word end
		char *t2 = t;
		while (t2[0] != ' ' && t2[0] != '\0') {
			t2++;
		}
		p->width = t2 -t;

		// add each letter of the word
		int characterNumber = 0;
		for (int i = 0; i < p->width; i++) {

			struct dd_meshTexture m2;
			dd_meshTexture_create(&m2);
			dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);
			dd_meshColour_set_colour(&m2, 1, 1, 1);

			// for each letter, create a mesh and position it
			int offsetX = (t[i] -32) %fontColumns;
			int offsetY = (fontColumns-1) -((t[i] -32) /fontColumns);
			dd_meshTexture_set_primitive_texcoords(&m2,
				((fontWidth /fontColumns) *offsetX),
				(1.0 -fontHeight) +((fontHeight /fontRows) *offsetY),
				(fontWidth /fontColumns),
				(fontHeight /fontRows)
			);

			dd_meshTexture_combine(&p->m, &m2, fontKerning/2 +characterNumber *fontKerning, 0, 0);

			// move to next character
			characterNumber++;

		}

		t += p->width;

	} while (t[0] != '\0');

	#endif

}

void dd_string3d_setKerning(float nkerning) {
	fontKerning = nkerning;
}
