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

	avdl_dynamic_array_init(&o->textMeshes, sizeof(struct dd_word_mesh));

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

	o->clean = dd_string3d_clean;
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
	for (int i = 0; i < avdl_dynamic_array_count(&o->textMeshes); i++) {
		struct dd_word_mesh *p;
		p = avdl_dynamic_array_get(&o->textMeshes, i);
		avdl_mesh_clean(&p->m);
		if (o->openglContextId == o->font->openglContextId) {
			for (int j = 0; j < p->length; j++) {
				avdl_font_releaseGlyph(o->font, p->glyph_ids[j]);
			}
		}
		p->length = 0;
	}
	avdl_dynamic_array_empty(&o->textMeshes);
}

void dd_string3d_drawInt(struct dd_string3d *o, int num) {

	if (!o->font) {
		avdl_log("avdl: dd_string3d_drawInt: no font");
		return;
	}

	#ifndef AVDL_DIRECT3D11

	// drawing ints is special
	if (!o->is_int) {
		avdl_log("string3d configured as text, but trying to draw int");
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
		struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, numberString[i] -'0');
		lineWidth += m->widthf;
	}
	// add `.` every 3 digits
	struct dd_word_mesh *mDot = avdl_dynamic_array_get(&o->textMeshes, 10);
	lineWidth += (mDot->widthf *(num_len /3));

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
		struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, numberString[i] -'0');

		int previousProgram;
		previousProgram = avdl_graphics_GetCurrentProgram();
		avdl_graphics_UseProgram(defaultProgram);
		GLint MatrixID = avdl_graphics_GetUniformLocation(defaultProgram, "matrix");
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

		avdl_mesh_draw(&m->m);

		dd_translatef(m->widthf, 0, 0);

		// draw `.` every 3 digits
		if ((num_len -i -1) %3 == 0 && (num_len -i -1) != 0) {
			avdl_mesh_draw(&mDot->m);
			dd_translatef(mDot->widthf, 0, 0);
		}

		avdl_graphics_UseProgram(previousProgram);
	}

	dd_matrix_pop();
	#endif
}

void dd_string3d_drawIntPadded(struct dd_string3d *o, int num, int digits) {
	#ifndef AVDL_DIRECT3D11

	// drawing ints is special
	if (!o->is_int) {
		avdl_log("string3d configured as text, but trying to draw int");
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
	int padding = dd_math_max(digits -num_len, 0);
	float lineWidth = 0;
	for (int i = 0; i < num_len +padding; i++) {
		if (i < padding) {
			struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, 0);
			lineWidth += m->widthf;
		}
		else {
			struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, numberString[i -padding] -'0');
			lineWidth += m->widthf;
		}
	}
	// add `.` every 3 digits
	struct dd_word_mesh *mDot = avdl_dynamic_array_get(&o->textMeshes, 10);
	lineWidth += (mDot->widthf *(num_len /3));

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

	for (int i = 0; i < num_len +padding; i++) {

		struct dd_word_mesh *m;
		if (i < padding) {
			m = avdl_dynamic_array_get(&o->textMeshes, 0);
		}
		else {
			m = avdl_dynamic_array_get(&o->textMeshes, numberString[i -padding] -'0');
		}

		int previousProgram;
		previousProgram = avdl_graphics_GetCurrentProgram();
		avdl_graphics_UseProgram(defaultProgram);
		GLint MatrixID = avdl_graphics_GetUniformLocation(defaultProgram, "matrix");
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());

		avdl_mesh_draw(&m->m);

		dd_translatef(m->widthf, 0, 0);

		// draw `.` every 3 digits
		if ((num_len -i -1) %3 == 0 && (num_len -i -1) != 0) {
			avdl_mesh_draw(&mDot->m);
			dd_translatef(mDot->widthf, 0, 0);
		}

		avdl_graphics_UseProgram(previousProgram);
	}

	dd_matrix_pop();
	#endif
}

void dd_string3d_drawLimit(struct dd_string3d *o, int limit) {
	dd_string3d_drawLimitTypewriter(o, limit, -1);
}

void dd_string3d_drawTypewriter(struct dd_string3d *o, int wordsToDraw) {
	dd_string3d_drawLimitTypewriter(o, 0, wordsToDraw);
}

void dd_string3d_drawLimitTypewriter(struct dd_string3d *o, int limit, int wordsToDraw) {

	if (!o->font) {
		avdl_log("avdl: dd_string3d_drawLimitTypewriter: no font");
		return;
	}

	if (o->is_int) {
		avdl_log("string3d configured as int, but trying to draw text");
	}

	dd_matrix_push();

	int wordsTotal = 0;
	int linesTotal = 0;
	int drawnWords = 0;

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

		for (int i = wordsTotal; i < avdl_dynamic_array_count(&o->textMeshes); i++) {
			struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, i);

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
	} while (wordsTotal < avdl_dynamic_array_count(&o->textMeshes));
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

		for (int i = wordsTotal; i < avdl_dynamic_array_count(&o->textMeshes); i++) {
			struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, i);

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

			if (wordsToDraw != -1 && drawnWords >= wordsToDraw) {
				break;
			}

			struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, wordsTotal +i);

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

			avdl_mesh_draw(&m->m);

			#if !defined( AVDL_DIRECT3D11 )
			avdl_graphics_UseProgram(previousProgram);
			#endif

			dd_translatef(m->widthf +m->space_size, 0, 0);

			drawnWords++;
		}
		wordsTotal += lineWords;
		dd_matrix_pop();
		dd_translatef(0, -1, 0);
	} while (wordsTotal < avdl_dynamic_array_count(&o->textMeshes));

	dd_matrix_pop();

}

void dd_string3d_clean(struct dd_string3d *o) {
	clean_words(o);
	avdl_dynamic_array_free(&o->textMeshes);
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

	const char *t = text;

	float space_size = SPACE_SIZE;

	do {

		// ignore whitespace
		while (!isunicode(t[0]) && (t[0] == ' ' || t[0] == '\t')) {
			t++;
		}

		// end of file
		if (!isunicode(t[0]) && t[0] == '\0') {
			break;
		}

		// create new mesh for the new word
		avdl_dynamic_array_push(&o->textMeshes, &m);
		p = avdl_dynamic_array_get(&o->textMeshes, avdl_dynamic_array_count(&o->textMeshes)-1);

		avdl_mesh_create(&p->m);

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
					avdl_mesh_set_primitive(&p->m, AVDL_PRIMITIVE_RECTANGLE);
					avdl_mesh_set_colour(&p->m, 0, 0, 0);

					struct avdl_texture *img = o->font->customIcon[i];
					avdl_mesh_setTexture(&p->m, img);
					avdl_mesh_setTransparency(&p->m, 1);
					//avdl_mesh_translatef(&p->m, 0.5, 0.1, 0); // TODO
					foundIcon = 1;
					break;
				}
			}
			if (foundIcon) {
				continue;
			}
		}

		avdl_mesh_setTexture(&p->m, &o->font->texture);
		avdl_mesh_setTransparency(&p->m, 1);

		int letters = 0;
		const char *tempptr = t;
		while ((!isunicode(tempptr[0]) && tempptr[0] != ' ' && tempptr[0] != '\0' && strncmp(tempptr, "\\n", 2) != 0) || isunicode(tempptr[0])) {
			if (isunicode(tempptr[0])) {
				int l = 0;
				utf8_decode(tempptr, &l);
				tempptr += l;
				letters += l;
			}
			else {
				tempptr++;
				letters++;
			}
		}
		//avdl_log("Letters: %d %s", letters, t);

		int verticesInRectangle = 6;
		int vcount = verticesInRectangle *letters;
		float *v = malloc(sizeof(float) *vcount *3);
		float *tex = malloc(sizeof(float) *vcount *2);
		//avdl_log("vcount: %d - v: %d", vcount, vcount *3);

		// add each letter of the word
		float advance = 0;
		int letterIndex = 0;
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

			struct avdl_mesh m3;
			avdl_mesh_create(&m3);
			avdl_mesh_set_primitive(&m3, AVDL_PRIMITIVE_RECTANGLE);

			float glyphWidth  = avdl_font_getGlyphWidth (o->font, glyph_id);
			float glyphHeight = avdl_font_getGlyphHeight(o->font, glyph_id);

			float offsetX = -(avdl_font_getGlyphWidth(o->font, glyph_id) /2)
					+avdl_font_getGlyphWidth(o->font, glyph_id)
					+avdl_font_getGlyphLeft(o->font, glyph_id)
					+advance;
			float offsetY = (avdl_font_getGlyphHeight(o->font, glyph_id) /2)
					-avdl_font_getGlyphHeight(o->font, glyph_id)
					+avdl_font_getGlyphTop(o->font, glyph_id);
			if (letterIndex *18 +18 >= vcount *3) {
				//avdl_log("    v (good): %d / %d", letterIndex *18 +17, vcount *3);
			}
			if (letterIndex *18 +17 >= vcount *3) {
				//avdl_log("    v: %d / %d", letterIndex *18 +17, vcount *3);
			}
			v[letterIndex *18 +0] = -0.5;
			v[letterIndex *18 +1] =  0.5;
			v[letterIndex *18 +2] = 0;

			v[letterIndex *18 +3] = -0.5;
			v[letterIndex *18 +4] = -0.5;
			v[letterIndex *18 +5] = 0;

			v[letterIndex *18 +6] =  0.5;
			v[letterIndex *18 +7] = -0.5;
			v[letterIndex *18 +8] = 0;

			v[letterIndex *18 + 9] =  0.5;
			v[letterIndex *18 +10] = -0.5;
			v[letterIndex *18 +11] = 0;

			v[letterIndex *18 +12] = 0.5;
			v[letterIndex *18 +13] = 0.5;
			v[letterIndex *18 +14] = 0;

			v[letterIndex *18 +15] = -0.5;
			v[letterIndex *18 +16] =  0.5;
			v[letterIndex *18 +17] = 0;

			if (o->isOnce) {

				if (letterIndex *12 +12 >= vcount *2) {
					//avdl_log("    t#1 (good): %d / %d", letterIndex *12 +11, vcount *2);
				}
				if (letterIndex *12 +11 >= vcount *2) {
					//avdl_log("    t#1: %d / %d", letterIndex *12 +11, vcount *2);
				}
				tex[letterIndex *12 +0] = 0;
				tex[letterIndex *12 +1] = 0;

				tex[letterIndex *12 +2] = 1;
				tex[letterIndex *12 +3] = 0;

				tex[letterIndex *12 +4] = 0;
				tex[letterIndex *12 +5] = 1;

				tex[letterIndex *12 +6] = 0;
				tex[letterIndex *12 +7] = 1;

				tex[letterIndex *12 +8] = 1;
				tex[letterIndex *12 +9] = 1;

				tex[letterIndex *12 +10] = 1;
				tex[letterIndex *12 +11] = 0;
			}
			else {
				float offsetX = avdl_font_getTexCoordX(o->font, glyph_id);
				float offsetY = avdl_font_getTexCoordY(o->font, glyph_id);
				#if defined( AVDL_DIRECT3D11 )
				offsetY = 1 -offsetY;
				#endif
				float sizeX = avdl_font_getTexCoordW(o->font, glyph_id);
				float sizeY = avdl_font_getTexCoordH(o->font, glyph_id);
				#if defined( AVDL_DIRECT3D11 )
				sizeY = -sizeY;
				#endif

				if ((letterIndex *12 +(5*2+2)) >= vcount *2) {
					//avdl_log("    t#2 (good): %d / %d", letterIndex *12 +(5*2+1), vcount *2);
				}
				if ((letterIndex *12 +(5*2+1)) >= vcount *2) {
					//avdl_log("    t#2: %d / %d", letterIndex *12 +(5*2+1), vcount *2);
				}
				for (int i = 0; i < 6; i++) {
					tex[letterIndex *12 +(i*2+0)] = (v[letterIndex *18 +(i*3+0)] +0.5) *sizeX +offsetX;
					tex[letterIndex *12 +(i*2+1)] = (v[letterIndex *18 +(i*3+1)] +0.5) *sizeY +offsetY;
				}
			}
			if (letterIndex *18 +15 +3 >= vcount *3) {
				//avdl_log("    v#2 (good): %d / %d", letterIndex *18 +17 +1, vcount *3);
			}
			if (letterIndex *18 +15 +1 >= vcount *3) {
				//avdl_log("    v#2: %d / %d", letterIndex *18 +17 +1, vcount *3);
			}
			for (int i = 0; i < 18; i += 3) {
				v[letterIndex *18 +i +0] *= glyphWidth;
				v[letterIndex *18 +i +1] *= glyphHeight;

				v[letterIndex *18 +i +0] += offsetX;
				v[letterIndex *18 +i +1] += offsetY;
			}

			avdl_mesh_clean(&m3);

			// move to next character
			advance += avdl_font_getGlyphAdvance(o->font, glyph_id);
			letterIndex++;

		}
		avdl_mesh_SetCustomData(&p->m, vcount, v, 0, tex);
		p->widthf = advance;

		avdl_mesh_set_colour(&p->m, 0, 0, 0);

	} while (t[0] != '\0');

}

void dd_string3d_setTextInt(struct dd_string3d *o) {
	o->is_int = 1;
	dd_string3d_setText(o, "0 1 2 3 4 5 6 7 8 9 .");
}


void dd_string3d_setFont(struct dd_string3d *o, struct avdl_font *font) {
	o->font = font;

	// text was set before given a font - init text now
	if (o->text) {
		dd_string3d_setText(o, o->text);
	}
}

float dd_string3d_getWidth(struct dd_string3d *o) {
	int linesTotal = 0;

	int lineWords = 0;
	float lineWidth = 0;
	linesTotal++;

	for (int i = 0; i < avdl_dynamic_array_count(&o->textMeshes); i++) {
		struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, i);

		// is newline character - stop parsing line
		if (m->is_newline) {
			lineWords++;
			break;
		}
		// in line
		else {
			// not first word, add space
			if (lineWords != 0) {
				lineWidth += m->space_size;
			}
			lineWidth += m->widthf;
			lineWords++;
		}
	}

	return lineWidth;
}

float dd_string3d_getWidthInt(struct dd_string3d *o, int num) {

	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	numberString[10] = '\0';

	int num_len = strlen(numberString);
	float lineWidth = 0;
	for (int i = 0; i < num_len; i++) {
		struct dd_word_mesh *m = avdl_dynamic_array_get(&o->textMeshes, numberString[i] -'0');
		lineWidth += m->widthf;
	}

	return lineWidth;
}

int dd_string3d_getWordCount(struct dd_string3d *o) {
	return avdl_dynamic_array_count(&o->textMeshes);
}
