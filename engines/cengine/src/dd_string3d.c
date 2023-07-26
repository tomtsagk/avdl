#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_cengine.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

#define SPACE_SIZE 0.5

void dd_string3d_create(struct dd_string3d *o) {

	dd_da_init(&o->textMeshes, sizeof(struct dd_word_mesh));

	o->align = DD_STRING3D_ALIGN_LEFT;
	o->alignv = DD_STRING3D_ALIGN_VERTICAL_TOP;
	o->colorFront[0] = 1.0;
	o->colorFront[1] = 1.0;
	o->colorFront[2] = 1.0;
	o->colorBack[0] = 0.0;
	o->colorBack[1] = 0.0;
	o->colorBack[2] = 0.0;
	o->font = 0;
	o->is_int = 0;

	o->text = 0;
	o->textw = 0;

	o->setAlign = dd_string3d_setAlign;
	o->setAlignVertical = dd_string3d_setAlignVertical;
	o->clean = dd_string3d_clean;
	o->draw = dd_string3d_draw;
	o->drawInt = dd_string3d_drawInt;
	o->drawLimit = dd_string3d_drawLimit;
	o->setText = dd_string3d_setText;
	o->setTextUnicode = dd_string3d_setTextUnicode;
	o->setTextInt = dd_string3d_setTextInt;
	o->setFont = dd_string3d_setFont;

}

void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al) {
	o->align = al;
}

void dd_string3d_setAlignVertical(struct dd_string3d *o, enum dd_string3d_align_vertical al) {
	o->alignv = al;
}

void dd_string3d_draw(struct dd_string3d *o) {
	dd_string3d_drawLimit(o, 0);
}

void dd_string3d_drawInt(struct dd_string3d *o, int num) {
	#ifndef AVDL_DIRECT3D11

	// drawing ints is special
	if (!o->is_int) {
		dd_log("string3d configured as text, but trying to draw int");
		return;
	}

	// no negatives yet!
	if (num < 0) {
		return;
	}

	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	numberString[10] = '\0';
	dd_matrix_push();

	int num_len = strlen(numberString);
	float lineWidth = 0;
	for (int i = 0; i < num_len; i++) {
		struct dd_word_mesh *m = dd_da_get(&o->textMeshes, numberString[i] -'0');
		lineWidth += m->widthf;
	}

	switch (o->align) {
	case DD_STRING3D_ALIGN_LEFT:
		break;
	case DD_STRING3D_ALIGN_CENTER:
		dd_translatef(-lineWidth *0.5, 0, 0);
		break;
	case DD_STRING3D_ALIGN_RIGHT:
		dd_translatef(-lineWidth, 0, 0);
		break;
	}

	for (int i = 0; i < num_len; i++) {
		struct dd_word_mesh *m = dd_da_get(&o->textMeshes, numberString[i] -'0');

		int previousProgram;
		previousProgram = avdl_graphics_GetCurrentProgram();
		avdl_graphics_UseProgram(defaultProgram);
		GLint MatrixID = avdl_graphics_GetUniformLocation(defaultProgram, "matrix");
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

		dd_meshTexture_draw(m);

		avdl_graphics_UseProgram(previousProgram);

		dd_translatef(m->widthf, 0, 0);
	}

	dd_matrix_pop();
	#endif
}

void dd_string3d_drawLimit(struct dd_string3d *o, int limit) {
	#ifndef AVDL_DIRECT3D11

	if (o->is_int) {
		dd_log("string3d configured as int, but trying to draw text");
	}

	dd_matrix_push();

	int wordsTotal = 0;
	int linesTotal = 0;

	// for each line
	do {
		int lineWords = 0;
		float lineWidth = 0;
		linesTotal++;

		for (int i = wordsTotal; i < o->textMeshes.elements; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, i);

			// fits in the same line
			if (!limit
			|| (!lineWords && m->widthf > limit)
			|| lineWidth +m->widthf +SPACE_SIZE <= limit) {
				// not first word, add space
				if (lineWords != 0) {
					lineWidth += SPACE_SIZE;
				}
				lineWidth += m->widthf;
				lineWords++;
			}
			// doesn't fit in line
			else {
				break;
			}
		}
		wordsTotal += lineWords;
	} while (wordsTotal < o->textMeshes.elements);
	switch (o->alignv) {
	case DD_STRING3D_ALIGN_VERTICAL_TOP:
		break;
	case DD_STRING3D_ALIGN_VERTICAL_CENTER:
		dd_translatef(0, ((linesTotal -1) *0.5), 0);
		break;
	case DD_STRING3D_ALIGN_VERTICAL_BOTTOM:
		dd_translatef(0, linesTotal -1, 0);
		break;
	}

	wordsTotal = 0;
	// for each line
	do {
		int lineWords = 0;
		float lineWidth = 0;

		for (int i = wordsTotal; i < o->textMeshes.elements; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, i);

			// fits in the same line
			if (!limit
			|| (!lineWords && m->widthf > limit)
			|| lineWidth +m->widthf +SPACE_SIZE <= limit) {
				// not first word, add space
				if (lineWords != 0) {
					lineWidth += SPACE_SIZE;
				}
				lineWidth += m->widthf;
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
			dd_translatef(-lineWidth *0.5, 0, 0);
			break;
		case DD_STRING3D_ALIGN_RIGHT:
			dd_translatef(-lineWidth, 0, 0);
			break;
		}

		for (int i = 0; i < lineWords; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, wordsTotal +i);

			int previousProgram;
			previousProgram = avdl_graphics_GetCurrentProgram();
			avdl_graphics_UseProgram(defaultProgram);
			GLint MatrixID = avdl_graphics_GetUniformLocation(defaultProgram, "matrix");
			avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

			dd_meshTexture_draw(&m->m);

			avdl_graphics_UseProgram(previousProgram);

			dd_translatef(m->widthf +SPACE_SIZE, 0, 0);
		}
		wordsTotal += lineWords;
		dd_matrix_pop();
		dd_translatef(0, -1, 0);
	} while (wordsTotal < o->textMeshes.elements);

	dd_matrix_pop();
	#endif
}

void dd_string3d_clean(struct dd_string3d *o) {
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
		for (int j = 0; j < p->width; j++) {
			avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
		}
	}
	dd_da_empty(&o->textMeshes);
}

// dirty hack to look at the atlas
int once = 1;

void dd_string3d_setText(struct dd_string3d *o, const char *text) {

	o->text = text;

	if (!o->font) {
		return;
	}

	#ifndef AVDL_DIRECT3D11
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
		for (int j = 0; j < p->width; j++) {
			avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
		}
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
		dd_meshTexture_setTexture(&p->m, &o->font->texture);
		dd_meshTexture_setTransparency(&p->m, 1);

		// find characters until word end
		char *t2 = t;
		while (t2[0] != ' ' && t2[0] != '\0') {
			t2++;
		}
		p->width = t2 -t;
		if (p->width > 100) {
			dd_log("currently a word can have maximum 100 characters");
			continue;
		}

		// add each letter of the word
		int characterNumber = 0;
		float advance = 0;
		for (int i = 0; i < p->width; i++) {

			int glyph_id = avdl_font_registerGlyph(o->font, t[i]);
			if (glyph_id == -1) {
				continue;
			}

			p->glyph_ids[i] = glyph_id;

			struct dd_meshTexture m2;
			dd_meshTexture_create(&m2);
			dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);

			int error;

			if (once) {
				dd_meshTexture_set_primitive_texcoords(&m2, 0, 0, 1, 1);
				once = 0;
			}
			else {
			/*
			*/
				dd_meshTexture_set_primitive_texcoords(&m2,
					avdl_font_getTexCoordX(o->font, glyph_id),
					avdl_font_getTexCoordY(o->font, glyph_id),
					avdl_font_getTexCoordW(o->font, glyph_id),
					avdl_font_getTexCoordH(o->font, glyph_id)
				);
			}
			/*
			*/

			dd_mesh_scalef(&m2,
				avdl_font_getGlyphWidth (o->font, glyph_id),
				avdl_font_getGlyphHeight(o->font, glyph_id),
				1
			);

			dd_meshTexture_combine(&p->m, &m2,
				-(avdl_font_getGlyphWidth(o->font, glyph_id) /2)
					+avdl_font_getGlyphWidth(o->font, glyph_id)
					+avdl_font_getGlyphLeft(o->font, glyph_id)
					+advance,
				(avdl_font_getGlyphHeight(o->font, glyph_id) /2)
					-avdl_font_getGlyphHeight(o->font, glyph_id)
					+avdl_font_getGlyphTop(o->font, glyph_id),
				0
			);
			dd_meshTexture_clean(&m2);

			// move to next character
			characterNumber++;
			advance += avdl_font_getGlyphAdvance(o->font, glyph_id);

		}
		p->widthf = advance;

		dd_meshColour_set_colour(&p->m, 0, 0, 0);

		t += p->width;

	} while (t[0] != '\0');

	#endif
}

void dd_string3d_setTextUnicode(struct dd_string3d *o, const wchar_t *text) {

	o->textw = text;

	if (!o->font) {
		return;
	}

	#ifndef AVDL_DIRECT3D11
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
		for (int j = 0; j < p->width; j++) {
			avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
		}
	}
	dd_da_empty(&o->textMeshes);

	// add new text meshes
	struct dd_word_mesh m;
	struct dd_word_mesh *p;

	wchar_t *t = text;

	do {
		// skip whitespace
		if (t[0] == L' ') {
			t++;
			continue;
		}

		// create new mesh for the new word
		dd_da_add(&o->textMeshes, &m);
		p = dd_da_get(&o->textMeshes, o->textMeshes.elements-1);

		dd_meshTexture_create(&p->m);
		dd_meshTexture_setTexture(&p->m, &o->font->texture);
		dd_meshTexture_setTransparency(&p->m, 1);

		// find characters until word end
		wchar_t *t2 = t;
		while (t2[0] != L' ' && t2[0] != L'\0') {
			t2++;
		}
		p->width = t2 -t;
		if (p->width > 100) {
			dd_log("currently a word can have maximum 100 characters");
			continue;
		}

		// add each letter of the word
		int characterNumber = 0;
		float advance = 0;
		for (int i = 0; i < p->width; i++) {

			int glyph_id = avdl_font_registerGlyph(o->font, t[i]);
			if (glyph_id == -1) {
				p->glyph_ids[i] = -1;
				continue;
			}

			p->glyph_ids[i] = glyph_id;

			struct dd_meshTexture m2;
			dd_meshTexture_create(&m2);
			dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);

			int error;

			if (once) {
				dd_meshTexture_set_primitive_texcoords(&m2, 0, 0, 1, 1);
				once = 0;
			}
			else {
			/*
			*/
				dd_meshTexture_set_primitive_texcoords(&m2,
					avdl_font_getTexCoordX(o->font, glyph_id),
					avdl_font_getTexCoordY(o->font, glyph_id),
					avdl_font_getTexCoordW(o->font, glyph_id),
					avdl_font_getTexCoordH(o->font, glyph_id)
				);
			}
			/*
			*/

			dd_mesh_scalef(&m2,
				avdl_font_getGlyphWidth (o->font, glyph_id),
				avdl_font_getGlyphHeight(o->font, glyph_id),
				1
			);

			dd_meshTexture_combine(&p->m, &m2,
				-(avdl_font_getGlyphWidth(o->font, glyph_id) /2)
					+avdl_font_getGlyphWidth(o->font, glyph_id)
					+avdl_font_getGlyphLeft(o->font, glyph_id)
					+advance,
				(avdl_font_getGlyphHeight(o->font, glyph_id) /2)
					-avdl_font_getGlyphHeight(o->font, glyph_id)
					+avdl_font_getGlyphTop(o->font, glyph_id),
				0
			);
			dd_meshTexture_clean(&m2);

			// move to next character
			characterNumber++;
			advance += avdl_font_getGlyphAdvance(o->font, glyph_id);

		}
		p->widthf = advance;
		dd_meshColour_set_colour(&p->m, 0, 0, 0);

		t += p->width;

	} while (t[0] != '\0');

	#endif

}

void dd_string3d_setTextInt(struct dd_string3d *o) {

	o->is_int = 1;

	if (!o->font) {
		return;
	}

	#ifndef AVDL_DIRECT3D11
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
		for (int j = 0; j < p->width; j++) {
			avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
		}
	}
	dd_da_empty(&o->textMeshes);

	// add new text meshes
	struct dd_word_mesh m;
	struct dd_word_mesh *p;

	char *text = "0 1 2 3 4 5 6 7 8 9";
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
		dd_meshTexture_setTexture(&p->m, &o->font->texture);
		dd_meshTexture_setTransparency(&p->m, 1);

		// find characters until word end
		char *t2 = t;
		while (t2[0] != ' ' && t2[0] != '\0') {
			t2++;
		}
		p->width = t2 -t;
		if (p->width > 100) {
			dd_log("currently a word can have maximum 100 characters");
			continue;
		}

		// add each letter of the word
		int characterNumber = 0;
		float advance = 0;
		for (int i = 0; i < p->width; i++) {

			int glyph_id = avdl_font_registerGlyph(o->font, t[i]);
			if (glyph_id == -1) {
				continue;
			}

			p->glyph_ids[i] = glyph_id;

			struct dd_meshTexture m2;
			dd_meshTexture_create(&m2);
			dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);

			int error;

			if (once) {
				dd_meshTexture_set_primitive_texcoords(&m2, 0, 0, 1, 1);
				once = 0;
			}
			else {
			/*
			*/
				dd_meshTexture_set_primitive_texcoords(&m2,
					avdl_font_getTexCoordX(o->font, glyph_id),
					avdl_font_getTexCoordY(o->font, glyph_id),
					avdl_font_getTexCoordW(o->font, glyph_id),
					avdl_font_getTexCoordH(o->font, glyph_id)
				);
			}
			/*
			*/

			dd_mesh_scalef(&m2,
				avdl_font_getGlyphWidth (o->font, glyph_id),
				avdl_font_getGlyphHeight(o->font, glyph_id),
				1
			);

			dd_meshTexture_combine(&p->m, &m2,
				-(avdl_font_getGlyphWidth(o->font, glyph_id) /2)
					+avdl_font_getGlyphWidth(o->font, glyph_id)
					+avdl_font_getGlyphLeft(o->font, glyph_id)
					+advance,
				(avdl_font_getGlyphHeight(o->font, glyph_id) /2)
					-avdl_font_getGlyphHeight(o->font, glyph_id)
					+avdl_font_getGlyphTop(o->font, glyph_id),
				0
			);
			dd_meshTexture_clean(&m2);

			// move to next character
			characterNumber++;
			advance += avdl_font_getGlyphAdvance(o->font, glyph_id);

		}
		p->widthf = advance;

		dd_meshColour_set_colour(&p->m, 0, 0, 0);

		t += p->width;

	} while (t[0] != '\0');

	#endif
}


void dd_string3d_setFont(struct dd_string3d *o, struct avdl_font *font) {
	o->font = font;

	if (o->text) {
		dd_string3d_setText(o, o->text);
	}
	else
	if (o->textw) {
		dd_string3d_setTextUnicode(o, o->textw);
	}
	else
	if (o->is_int) {
		dd_string3d_setTextInt(o);
	}
}
