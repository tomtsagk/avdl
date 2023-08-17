#ifndef AVDL_FONT_H
#define AVDL_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_MAX_GLYPHS_ROWS 22
#define FONT_MAX_GLYPHS_COLUMNS 22
#define FONT_MAX_GLYPHS 484

struct avdl_glyph {
	int uses;
	int id;
	float texcoordW;
	float texcoordH;
	float left;
	float top;
	float width;
	float height;
	float advance;
};

/*
 * designed to load a font from a file and render desired glyphs.
 */
struct avdl_font {
	struct dd_image texture;
	FT_Face face;
	int outline_thickness;

	FT_Byte *fontData;

	struct avdl_glyph glyphs[FONT_MAX_GLYPHS];

	int openglContextId;

	// custom icons for drawing special icons in the middle of the font
	struct dd_image *customIcon[10];
	const char *customIconKeyword[10];
	int customIconCount;

	void (*clean)(struct avdl_font *);
	void (*set)(struct avdl_font *, const char *name, int filetype, int outline_thickness);
	void (*addCustomIcon)(struct avdl_font *, const char *keyword, struct dd_image *);
};

int avdl_font_init();
int avdl_font_deinit();

void avdl_font_create(struct avdl_font *);
void avdl_font_clean(struct avdl_font *);

void avdl_font_set(struct avdl_font *, const char *name, int filetype, int outline_thickness);

int avdl_font_registerGlyph(struct avdl_font *, int);
int avdl_font_releaseGlyph(struct avdl_font *, int);
int avdl_font_releaseAllGlyphs(struct avdl_font *);

float avdl_font_getTexCoordX(struct avdl_font *, int);
float avdl_font_getTexCoordY(struct avdl_font *, int);
float avdl_font_getTexCoordW(struct avdl_font *, int);
float avdl_font_getTexCoordH(struct avdl_font *, int);

float avdl_font_getGlyphWidth(struct avdl_font *, int);
float avdl_font_getGlyphHeight(struct avdl_font *, int);
float avdl_font_getGlyphLeft(struct avdl_font *, int);
float avdl_font_getGlyphTop(struct avdl_font *, int);

float avdl_font_getGlyphAdvance(struct avdl_font *, int);

int avdl_font_needsRefresh(struct avdl_font *);

void avdl_font_addCustomIcon(struct avdl_font *, const char *keyword, struct dd_image *);

#ifdef __cplusplus
}
#endif

#endif
