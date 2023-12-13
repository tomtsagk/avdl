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

	o->isOnce = 0;

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

static void clean_words(struct dd_string3d *o) {
	// empty previous text meshes (if any)
	for (int i = 0; i < o->textMeshes.elements; i++) {
		struct dd_word_mesh *p;
		p = dd_da_get(&o->textMeshes, i);
		dd_meshTexture_clean(&p->m);
		if (o->openglContextId == o->font->openglContextId) {
			for (int j = 0; j < p->length; j++) {
				avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
			}
		}
		p->length = 0;
	}
	dd_da_empty(&o->textMeshes);
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

	if (o->font && (avdl_font_needsRefresh(o->font) || o->openglContextId != o->font->openglContextId)) {
		if (o->is_int) {
			dd_string3d_setTextInt(o);
		}
		o->openglContextId = o->font->openglContextId;
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

	if (o->is_int) {
		dd_log("string3d configured as int, but trying to draw text");
	}

	dd_matrix_push();

	int wordsTotal = 0;
	int linesTotal = 0;

	if (o->font && (avdl_font_needsRefresh(o->font) || o->openglContextId != o->font->openglContextId)) {
		if (o->text) {
			dd_string3d_setText(o, o->text);
		}
		o->openglContextId = o->font->openglContextId;
	}

	// for each line
	do {
		int lineWords = 0;
		float lineWidth = 0;
		linesTotal++;

		for (int i = wordsTotal; i < o->textMeshes.elements; i++) {
			struct dd_word_mesh *m = dd_da_get(&o->textMeshes, i);

			// is newline character - stop parsing line
			if (m->is_newline) {
				lineWords++;
				break;
			}
			else
			// fits in the same line
			if (!limit
			|| !lineWords
			|| lineWidth +m->widthf +m->space_size <= limit) {
				// not first word, add space
				if (lineWords != 0) {
					lineWidth += m->space_size;
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

			// newline character - end line
			if (m->is_newline) {
				lineWords++;
				break;
			}
			else
			// fits in the same line
			if (!limit
			|| !lineWords
			|| lineWidth +m->widthf +m->space_size <= limit) {
				// not first word, add space
				if (lineWords != 0) {
					lineWidth += m->space_size;
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

			if (m->is_newline) {
				break;
			}

			#if !defined( AVDL_DIRECT3D11 )
			int previousProgram;
			previousProgram = avdl_graphics_GetCurrentProgram();
			avdl_graphics_UseProgram(defaultProgram);
			GLint MatrixID = avdl_graphics_GetUniformLocation(defaultProgram, "matrix");
			avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
			#endif

			dd_meshTexture_draw(&m->m);

			#if !defined( AVDL_DIRECT3D11 )
			avdl_graphics_UseProgram(previousProgram);
			#endif

			dd_translatef(m->widthf +m->space_size, 0, 0);
		}
		wordsTotal += lineWords;
		dd_matrix_pop();
		dd_translatef(0, -1, 0);
	} while (wordsTotal < o->textMeshes.elements);

	dd_matrix_pop();

}

void dd_string3d_clean(struct dd_string3d *o) {
	clean_words(o);
	dd_da_free(&o->textMeshes);
}

// if it uses bits 0x1100_0000 - it's unicode
#define isunicode(c) ( ( (c) & 0xc0) == 0xc0 )

static int utf8_decode(const char *str, int *i) {
	const unsigned char *s = (const unsigned char *)str;
	int u = *s;
	int l = 1;
	if( isunicode(u) ) {
		int a = (u&0x20)? ((u&0x10)? ((u&0x08)? ((u&0x04)? 6 : 5) : 4) : 3) : 2;
		if( a < 6 || !(u&0x02) ) {
			u = ( ( u << (a+1) ) & 0xff ) >> (a+1);
			for(int b = 1; b < a; ++b) {
				u = ( u << 6 ) | ( s[l++] & 0x3f );
			}
		}
	}
	if (i) {
		*i += l;
	}
	return u;
}

void dd_string3d_setText(struct dd_string3d *o, const char *text) {

	o->text = text;

	if (!o->font) {
		return;
	}

	clean_words(o);
	o->openglContextId = o->font->openglContextId;

	// add new text meshes
	struct dd_word_mesh m;
	m.is_newline = 0;
	struct dd_word_mesh *p;

	char *t = text;

	float space_size = SPACE_SIZE;

	int length;
	do {
		length = 0;

		// ignore whitespace
		while (!isunicode(t[0]) && (t[0] == ' ' || t[0] == '\t')) {
			t++;
		}

		// end of file
		if (!isunicode(t[0]) && t[0] == '\0') {
			break;
		}

		// create new mesh for the new word
		dd_da_push(&o->textMeshes, &m);
		p = dd_da_get(&o->textMeshes, o->textMeshes.elements-1);

		dd_meshTexture_create(&p->m);

		// find characters until word end
		p->length = 0;

		// newline
		if (strncmp(t, "\\n", 2) == 0) {
			t += 2;
			p->is_newline = 1;
			continue;
		}

		// no spaces
		if (strncmp(t, "\\j", 2) == 0) {
			t += 2;
			space_size = 0;
		}
		p->space_size = space_size;

		// special symbols
		if (o->font->customIconCount > 0) {
			int foundIcon = 0;
			for (int i = 0; i < o->font->customIconCount; i++) {
				if (strncmp(t, o->font->customIconKeyword[i], strlen(o->font->customIconKeyword[i])) == 0) {
					t += strlen(o->font->customIconKeyword[i]);
					p->widthf = 0.9;
					dd_meshTexture_set_primitive(&p->m, DD_PRIMITIVE_RECTANGLE);
					dd_meshColour_set_colour(&p->m, 0, 0, 0);

					struct dd_image *img = o->font->customIcon[i];
					dd_meshTexture_setTexture(&p->m, img);
					dd_meshTexture_setTransparency(&p->m, 1);
					dd_mesh_translatef(&p->m, 0.5, 0.1, 0);
					foundIcon = 1;
					break;
				}
			}
			if (foundIcon) {
				continue;
			}
		}

		dd_meshTexture_setTexture(&p->m, &o->font->texture);
		dd_meshTexture_setTransparency(&p->m, 1);

		// add each letter of the word
		float advance = 0;
		while ((!isunicode(t[0]) && t[0] != ' ' && t[0] != '\0' && strncmp(t, "\\n", 2) != 0) || isunicode(t[0])) {
			p->length++;

			int charid = 0;
			if (isunicode(t[0])) {
				int l = 0;
				charid = utf8_decode(t, &l);
				t += l;
			}
			else {
				charid = t[0];
				t++;
			}

			int glyph_id = avdl_font_registerGlyph(o->font, charid);
			if (glyph_id == -1) {
				continue;
			}

			p->glyph_ids[p->length -1] = glyph_id;

			struct dd_meshTexture m2;
			dd_meshTexture_create(&m2);
			dd_meshTexture_set_primitive(&m2, DD_PRIMITIVE_RECTANGLE);

			int error;

			if (o->isOnce) {
				dd_meshTexture_set_primitive_texcoords(&m2, 0, 0, 1, 1);
			}
			else {
				dd_meshTexture_set_primitive_texcoords(&m2,
					avdl_font_getTexCoordX(o->font, glyph_id),
					#if defined( AVDL_DIRECT3D11 )
					1 - avdl_font_getTexCoordY(o->font, glyph_id),
					#else
					avdl_font_getTexCoordY(o->font, glyph_id),
					#endif
					avdl_font_getTexCoordW(o->font, glyph_id),
					#if defined( AVDL_DIRECT3D11 )
					-avdl_font_getTexCoordH(o->font, glyph_id)
					#else
					avdl_font_getTexCoordH(o->font, glyph_id)
					#endif
				);
			}

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
			advance += avdl_font_getGlyphAdvance(o->font, glyph_id);

		}
		p->widthf = advance;

		dd_meshColour_set_colour(&p->m, 0, 0, 0);

	} while (t[0] != '\0');

}

void dd_string3d_setTextInt(struct dd_string3d *o) {
	o->is_int = 1;
	dd_string3d_setText(o, "0 1 2 3 4 5 6 7 8 9");
}


void dd_string3d_setFont(struct dd_string3d *o, struct avdl_font *font) {
	o->font = font;

	// text was set before given a font - init text now
	if (o->text) {
		dd_string3d_setText(o, o->text);
	}
}
