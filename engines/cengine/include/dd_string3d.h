#ifndef DD_TEXT_H
#define DD_TEXT_H

#include "avdl_cengine.h"
#include "avdl_texture.h"
#include "dd_meshTexture.h"
#include "avdl_font.h"

#ifdef __cplusplus
extern "C" {
#endif

enum dd_string3d_align {
	DD_STRING3D_ALIGN_LEFT,
	DD_STRING3D_ALIGN_CENTER,
	DD_STRING3D_ALIGN_RIGHT,
};

enum dd_string3d_align_vertical {
	DD_STRING3D_ALIGN_VERTICAL_TOP,
	DD_STRING3D_ALIGN_VERTICAL_CENTER,
	DD_STRING3D_ALIGN_VERTICAL_BOTTOM,
};

struct dd_word_mesh {
	struct dd_meshTexture m;
	int glyph_ids[100];
	int length;
	float widthf;
	int is_newline;
	float space_size;
};

struct dd_string3d {

	struct avdl_dynamic_array textMeshes;

	// Align
	enum dd_string3d_align align;
	enum dd_string3d_align_vertical alignv;

	float colorFront[3];
	float colorBack[3];

	int len;
	const char *text;
	const wchar_t *textw;

	struct avdl_font *font;

	int isOnce;
	int is_int;

	int openglContextId;

	void (*clean)(struct dd_string3d *);
};

void dd_string3d_create(struct dd_string3d *o);
void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al);
void dd_string3d_setAlignVertical(struct dd_string3d *o, enum dd_string3d_align_vertical al);

void dd_string3d_draw(struct dd_string3d *o);
void dd_string3d_drawInt(struct dd_string3d *o, int num);
void dd_string3d_drawIntPadded(struct dd_string3d *o, int num, int digits);
void dd_string3d_drawLimit(struct dd_string3d *o, int limit);
void dd_string3d_drawLimitTypewriter(struct dd_string3d *o, int limit, int wordsToDraw);
void dd_string3d_drawTypewriter(struct dd_string3d *o, int wordsToDraw);

void dd_string3d_clean(struct dd_string3d *o);

void dd_string3d_setText(struct dd_string3d *o, const char *text);
void dd_string3d_setTextInt(struct dd_string3d *o);

void dd_string3d_setFont(struct dd_string3d *, struct avdl_font *);

float dd_string3d_getWidth(struct dd_string3d *);
float dd_string3d_getWidthInt(struct dd_string3d *, int);

#ifdef __cplusplus
}
#endif

#endif
