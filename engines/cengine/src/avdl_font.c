#include "avdl_font.h"
#include "dd_math.h"
#include "dd_log.h"

#if !defined( AVDL_DIRECT3D11 )
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

FT_Library library;
#endif

#define FONT_ATLAS_WIDTH 2048
#define FONT_ATLAS_HEIGHT 2048

#define FONT_GLYPH_SIZE 90

int avdl_font_init() {

	#ifndef AVDL_DIRECT3D11
	int error = FT_Init_FreeType( &library );
	if (error) {
		dd_log("avdl: error initialising freetype2");
		return -1;
	}

	#endif
	return 0;

} // string3d init

int avdl_font_deinit() {

	#if !defined( AVDL_DIRECT3D11 )
	if (library) {
		FT_Done_FreeType( library );
	}
	#endif
	return 0;
}

void avdl_font_create(struct avdl_font *o) {

	// functions
	o->set = avdl_font_set;
	o->addCustomIcon = avdl_font_addCustomIcon;

	#ifndef AVDL_DIRECT3D11
	o->customIconCount = 0;

	for (int i = 0; i < FONT_MAX_GLYPHS; i++) {
		o->glyphs[i].uses = 0;
	}

	dd_image_create(&o->texture);
	o->texture.width = FONT_ATLAS_WIDTH;
	o->texture.height = FONT_ATLAS_HEIGHT;
	o->texture.pixelFormat = GL_RGBA;
	o->texture.openglContextId = avdl_graphics_getContextId();
	o->outline_thickness = 0;
	o->fontData = 0;

	o->openglContextId = o->texture.openglContextId;
	#endif

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	o->texture.pixelsb = malloc(sizeof(GLubyte) *4 *o->texture.width *o->texture.height);

	// clean the texture
	for (int x = 0; x < o->texture.width ; x++)
	for (int y = 0; y < o->texture.height; y++) {
		o->texture.pixelsb[(y*o->texture.width*4) +x*4+0] = 0;
		o->texture.pixelsb[(y*o->texture.width*4) +x*4+1] = 0;
		o->texture.pixelsb[(y*o->texture.width*4) +x*4+2] = 0;
		o->texture.pixelsb[(y*o->texture.width*4) +x*4+3] = 0;
	}

	#else

	#ifndef AVDL_DIRECT3D11
	o->texture.pixels = malloc(sizeof(float) *4 *o->texture.width *o->texture.height);

	// clean the texture
	for (int x = 0; x < o->texture.width ; x++)
	for (int y = 0; y < o->texture.height; y++) {
		o->texture.pixels[(y*o->texture.width*4) +x*4+0] = 1;
		o->texture.pixels[(y*o->texture.width*4) +x*4+1] = 1;
		o->texture.pixels[(y*o->texture.width*4) +x*4+2] = 1;
		o->texture.pixels[(y*o->texture.width*4) +x*4+3] = 0;
	}
	#endif

	#endif

	#if !defined( AVDL_DIRECT3D11 )
	o->face = 0;
	#endif

}

static void CleanFontData(struct avdl_font *o) {
	#ifndef AVDL_DIRECT3D11
	if (o->fontData) {
		free(o->fontData);
	}
	#endif
}

static void CleanFontFace(struct avdl_font *o) {
	#if !defined( AVDL_DIRECT3D11 )
	if (o->face) {
		FT_Done_Face(o->face);
		o->face = 0;
	}
	#endif
}

void avdl_font_clean(struct avdl_font *o) {
	#if !defined( AVDL_DIRECT3D11 )
	dd_image_clean(&o->texture);

	CleanFontData(o);
	CleanFontFace(o);
	#endif
}

int avdl_font_registerGlyph(struct avdl_font *o, int unicode_hex) {

	#if defined( AVDL_DIRECT3D11 )
	return -1;
	#else

	if (!o->face) {
		return -1;
	}

	// check if glyph exists
	for (int i = 0; i < FONT_MAX_GLYPHS; i++) {
		if (o->glyphs[i].uses > 0 && unicode_hex == o->glyphs[i].id) {
			o->glyphs[i].uses++;
			return i;
		}
	}

	// find a free slot
	int glyph_id = -1;
	for (int i = 0; i < FONT_MAX_GLYPHS; i++) {
		if (o->glyphs[i].uses <= 0) {
			glyph_id = i;
			break;
		}
	}
	if (glyph_id == -1) {
		dd_log("avdl error: cannot make more glyphs for this font");
		return -1;
	}

	FT_Error error = FT_Set_Pixel_Sizes(
		o->face,
		0,
		FONT_GLYPH_SIZE -20
	);
	if (error) {
		dd_log("avdl error: could not set pixel size for font");
	}

	int pixels_size = FONT_GLYPH_SIZE *FONT_GLYPH_SIZE *4;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	GLubyte pixels[pixels_size];
	for (int i = 0; i < pixels_size; i++) {
		pixels[i] = 0;
	}
	#else
	//float pixels[pixels_size];
	float *pixels = malloc(sizeof(float) *pixels_size);
	for (int i = 0; i < pixels_size; i++) {
		pixels[i] = 0.0;
	}
	#endif

	// outline
	int cx = 0;
	int cy = 0;
	FT_Glyph glyphDescStroke;
	if (o->outline_thickness > 0) {
		FT_Stroker stroker;
		FT_Stroker_New(library, &stroker);
		error = FT_Load_Char( o->face, unicode_hex, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL );
		if ( error != 0 ) {
			dd_log("avdl: error calling `FT_Load_Char`");
		}

		error = FT_Get_Glyph( o->face->glyph, &glyphDescStroke );
		if ( error ) {
			dd_log("avdl: error calling `FT_Get_Glyph`");
		}

		FT_Stroker_Set( stroker, o->outline_thickness * (float)(1 << 6), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

		error = FT_Glyph_Stroke( &glyphDescStroke, stroker, 1 );
		if ( error ) {
			dd_log("avdl: error calling `FT_Glyph_Stroke`");
		}

		error = FT_Glyph_To_Bitmap( &glyphDescStroke, FT_RENDER_MODE_NORMAL, 0, 1);
		if ( error ) {
			dd_log("avdl: error calling `FT_Glyph_To_Bitmap`");
		}

		FT_BitmapGlyph glyph_bitmap;
		FT_Bitmap *bitmap_stroke = 0;
		glyph_bitmap  = (FT_BitmapGlyph)glyphDescStroke;
		bitmap_stroke = &glyph_bitmap->bitmap;

		/*
		if (bitmap_stroke->width > FONT_GLYPH_SIZE) {
			dd_log("glyph is too wide: %d / %d", bitmap_stroke->width, FONT_GLYPH_SIZE);
		}
		if (bitmap_stroke->rows > FONT_GLYPH_SIZE) {
			dd_log("glyph is too tall: %d / %d", bitmap_stroke->rows, FONT_GLYPH_SIZE);
		}
		*/
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		// render glyph on texture
		for (int x = 0; x < bitmap_stroke->width; x++)
		for (int y = 0; y < bitmap_stroke->rows ; y++) {
			if (x >= FONT_GLYPH_SIZE) continue;
			if (y >= FONT_GLYPH_SIZE) continue;
			int ry = bitmap_stroke->rows-1 -y;
			int index = ry *FONT_GLYPH_SIZE *4 +x*4;
			/*
			if (index >= pixels_size) {
				dd_log("temp: %d %d %d - %d %d", y, FONT_GLYPH_SIZE, x, FONT_GLYPH_SIZE *4, x*4);
				dd_log("too much index: %d / %d, %dx%d", index, pixels_size, bitmap_stroke->width, bitmap_stroke->rows);
				continue;
			}
			*/
			pixels[index +0] = 0;
			pixels[index +1] = 0;
			pixels[index +2] = 0;
			pixels[index +3] = dd_math_min(bitmap_stroke->buffer[y*bitmap_stroke->width +x] *255, 255);
		}
		#else
		for (int x = 0; x < bitmap_stroke->width; x++)
		for (int y = 0; y < bitmap_stroke->rows ; y++) {
			if (x >= FONT_GLYPH_SIZE) continue;
			if (y >= FONT_GLYPH_SIZE) continue;
			int ry = bitmap_stroke->rows-1 -y;
			int index = ry *FONT_GLYPH_SIZE *4 +x*4;
			/*
			if (index >= pixels_size) {
				dd_log("temp: %d %d %d - %d %d", y, FONT_GLYPH_SIZE, x, FONT_GLYPH_SIZE *4, x*4);
				dd_log("too much index: %d / %d, %dx%d", index, pixels_size, bitmap_stroke->width, bitmap_stroke->rows);
				continue;
			}
			*/
			pixels[index +0] = 0;
			pixels[index +1] = 0;
			pixels[index +2] = 0;
			pixels[index +3] = bitmap_stroke->buffer[y*bitmap_stroke->width +x];
		}
		#endif
		cx = bitmap_stroke->width;
		cy = bitmap_stroke->rows;
		FT_Stroker_Done(stroker);
	}

	error = FT_Load_Char( o->face, unicode_hex, FT_LOAD_RENDER );
	if (error) {
		dd_log("avdl: error loading char glyph: %d", unicode_hex);
	}

	/*
	if (o->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
		//dd_log("TTF: BGRA");
	}
	else
	if (o->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
		//dd_log("TTF: mono");
	}
	else
	if (o->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
		//dd_log("TTF: gray");
	}
	else
	if (o->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY2) {
		//dd_log("TTF: gray2");
	}
	else
	if (o->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY4) {
		//dd_log("TTF: gray4");
	}
	else {
		//dd_log("TTF: other");
	}
	*/

	int cx_fill  = o->face->glyph->bitmap.width;
	int cy_fill  = o->face->glyph->bitmap.rows;
	int offset_x = (cx - cx_fill) / 2;
	int offset_y = (cy - cy_fill) / 2;

	// render glyph on texture
	for (int x = 0; x < o->face->glyph->bitmap.width; x++)
	for (int y = 0; y < o->face->glyph->bitmap.rows ; y++) {
		if (x >= FONT_GLYPH_SIZE) continue;
		if (y >= FONT_GLYPH_SIZE) continue;
		int ry2 = o->face->glyph->bitmap.rows-1 -y;
		if (o->outline_thickness > 0) {
			ry2 += offset_y;
		}
		int index2 = (ry2*FONT_GLYPH_SIZE*4) +x*4;
		if (o->outline_thickness > 0) {
			index2 += offset_x *4;
		}

		/*
		if (index2 >= pixels_size) {
			dd_log("temp2: %d %d %d", y, FONT_GLYPH_SIZE, x);
			dd_log("too much index2: %d / %d, %dx%d", index2, pixels_size, o->face->glyph->bitmap.width, o->face->glyph->bitmap.rows);
			continue;
		}
		*/
		float alpha = o->face->glyph->bitmap.buffer[y*o->face->glyph->bitmap.width +x];
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		pixels[index2+0] = dd_math_min(255, 255 *alpha);
		pixels[index2+1] = dd_math_min(255, 255 *alpha);
		pixels[index2+2] = dd_math_min(255, 255 *alpha);
		pixels[index2+3] = dd_math_max(pixels[index2+3], dd_math_min(alpha *255, 255));
		#else
		pixels[index2+0] = 1.0 *alpha;
		pixels[index2+1] = 1.0 *alpha;
		pixels[index2+2] = 1.0 *alpha;
		pixels[index2+3] = dd_math_max(pixels[index2+3], alpha);
		#endif
	}

	// pixels available, draw directly on them
	if (o->texture.pixelsb || o->texture.pixels) {
		for (int x = 0; x < FONT_GLYPH_SIZE; x++)
		for (int y = 0; y < FONT_GLYPH_SIZE; y++) {
			int ry = y;
			int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%FONT_MAX_GLYPHS_COLUMNS)*FONT_GLYPH_SIZE*4 +((glyph_id/FONT_MAX_GLYPHS_ROWS)*o->texture.width*4*FONT_GLYPH_SIZE);
			int indexPixel = y *FONT_GLYPH_SIZE *4 +x*4;
			#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			o->texture.pixelsb[index+0] = pixels[indexPixel +0];
			o->texture.pixelsb[index+1] = pixels[indexPixel +1];
			o->texture.pixelsb[index+2] = pixels[indexPixel +2];
			o->texture.pixelsb[index+3] = pixels[indexPixel +3];
			#else
			o->texture.pixels[index+0] = pixels[indexPixel +0];
			o->texture.pixels[index+1] = pixels[indexPixel +1];
			o->texture.pixels[index+2] = pixels[indexPixel +2];
			o->texture.pixels[index+3] = pixels[indexPixel +3];
			#endif
		}
	}
	else
	// texture already made, pass sub texture to draw
	if (o->texture.tex) {
		glBindTexture(GL_TEXTURE_2D, o->texture.tex);

		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		dd_image_addSubpixels(&o->texture, pixels, GL_RGBA,
			(glyph_id%FONT_MAX_GLYPHS_COLUMNS) *FONT_GLYPH_SIZE,
			((glyph_id/FONT_MAX_GLYPHS_ROWS) *FONT_GLYPH_SIZE),
			FONT_GLYPH_SIZE,
			FONT_GLYPH_SIZE
		);
		#else
		dd_image_addSubpixels(&o->texture, pixels, GL_FLOAT,
			(glyph_id%FONT_MAX_GLYPHS_COLUMNS) *FONT_GLYPH_SIZE,
			((glyph_id/FONT_MAX_GLYPHS_ROWS) *FONT_GLYPH_SIZE),
			FONT_GLYPH_SIZE,
			FONT_GLYPH_SIZE
		);
		#endif

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		//dd_log("dd_image has error state ?");
	}

	o->glyphs[glyph_id].id = unicode_hex;
	o->glyphs[glyph_id].uses++;

	if (o->outline_thickness > 0) {
		o->glyphs[glyph_id].texcoordW = cx /(float) FONT_ATLAS_WIDTH;
		o->glyphs[glyph_id].texcoordH = cy /(float) FONT_ATLAS_HEIGHT;
		o->glyphs[glyph_id].width  = cx /(float) FONT_GLYPH_SIZE;
		o->glyphs[glyph_id].height = cy  /(float) FONT_GLYPH_SIZE;

		o->glyphs[glyph_id].advance = ((o->face->glyph->advance.x >> 6) +o->outline_thickness) /(float) FONT_GLYPH_SIZE;

	}
	else {
		o->glyphs[glyph_id].texcoordW = o->face->glyph->bitmap.width /(float) FONT_ATLAS_WIDTH;
		o->glyphs[glyph_id].texcoordH = o->face->glyph->bitmap.rows  /(float) FONT_ATLAS_HEIGHT;
		o->glyphs[glyph_id].width  = o->face->glyph->bitmap.width /(float) FONT_GLYPH_SIZE;
		o->glyphs[glyph_id].height = o->face->glyph->bitmap.rows  /(float) FONT_GLYPH_SIZE;

		o->glyphs[glyph_id].advance = (o->face->glyph->advance.x >> 6) /(float) FONT_GLYPH_SIZE;
	}

	o->glyphs[glyph_id].left = o->face->glyph->bitmap_left /(float) FONT_GLYPH_SIZE;
	o->glyphs[glyph_id].top  = o->face->glyph->bitmap_top  /(float) FONT_GLYPH_SIZE;

	if (o->outline_thickness > 0) {
		FT_Done_Glyph( glyphDescStroke );
	}

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	#else
	free(pixels);
	#endif

	return glyph_id;

	#endif
}

int avdl_font_releaseGlyph(struct avdl_font *o, int glyph_id) {
	#if !defined( AVDL_DIRECT3D11 )
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return -1;
	}

	o->glyphs[glyph_id].uses = dd_math_max(o->glyphs[glyph_id].uses -1, 0);
	#endif

	return 0;
}

int avdl_font_releaseAllGlyphs(struct avdl_font *o) {
	#if !defined( AVDL_DIRECT3D11 )
	for (int i = 0; i < FONT_MAX_GLYPHS; i++) {
		o->glyphs[i].uses = 0;
	}
	#endif

	return 0;
}

float avdl_font_getTexCoordX(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return ((glyph_id%FONT_MAX_GLYPHS_COLUMNS)*FONT_GLYPH_SIZE) /(float) FONT_ATLAS_WIDTH;
	#endif
}

float avdl_font_getTexCoordY(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return ((glyph_id/FONT_MAX_GLYPHS_ROWS)*FONT_GLYPH_SIZE) /(float) FONT_ATLAS_HEIGHT;
	#endif
}

float avdl_font_getTexCoordW(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].texcoordW;
	#endif
}

float avdl_font_getTexCoordH(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].texcoordH;
	#endif
}

float avdl_font_getGlyphWidth(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].width;
	#endif
}

float avdl_font_getGlyphHeight(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].height;
	#endif
}

float avdl_font_getGlyphLeft(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].left;
	#endif
}

float avdl_font_getGlyphTop(struct avdl_font *o, int glyph_id) {
	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	if (o->glyphs[glyph_id].uses == 0) {
		return 0;
	}
	return o->glyphs[glyph_id].top;
	#endif
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <pthread.h>
extern pthread_mutex_t updateDrawMutex;
extern pthread_mutex_t jniMutex;
extern AAssetManager *aassetManager;
extern JNIEnv *jniEnv;
extern JavaVM* jvm;
extern jclass *clazz;
#endif

void avdl_font_set(struct avdl_font *o, const char *name, int filetype, int outline_thickness) {

	#if !defined( AVDL_DIRECT3D11 )
	CleanFontData(o);
	CleanFontFace(o);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	AAsset* fontFile = AAssetManager_open(aassetManager, name, AASSET_MODE_UNKNOWN);
	if (!fontFile) {
		dd_log("avdl error: unable to open font file: %s", name);
		return;
	}
	off_t fontDataSize = AAsset_getLength(fontFile);

	o->fontData = malloc(sizeof(FT_Byte) *fontDataSize);
	if (!o->fontData) {
		dd_log("avdl error: malloc failed when creating font");
		return;
	}
	AAsset_read(fontFile, o->fontData, fontDataSize);
	AAsset_close(fontFile);

	if (FT_New_Memory_Face(library, o->fontData, fontDataSize, 0, &o->face)) {
		dd_log("avdl error: load font from memory failed");
		o->face = 0;
	}
	#else

	// native

	int error = FT_New_Face(library,
		name,
		0,
		&o->face
	);
	if ( error == FT_Err_Unknown_File_Format ) {
		dd_log("avdl: unsupported font file format: %s", name);
		o->face = 0;
		return;
	}
	else if ( error ) {
		dd_log("avdl: unknown error parsing font: %s", name);
		o->face = 0;
		return;
	}
	#endif

	o->outline_thickness = outline_thickness;

	#endif

}

float avdl_font_getGlyphAdvance(struct avdl_font *o, int glyph_id) {
	#if !defined( AVDL_DIRECT3D11 )
	if (glyph_id < 0 || glyph_id >= FONT_MAX_GLYPHS) {
		return 0;
	}
	return o->glyphs[glyph_id].advance;
	#endif
}

int avdl_font_needsRefresh(struct avdl_font *o) {

	#if !defined( AVDL_DIRECT3D11 )
	// font needs to refresh
	if (o->openglContextId != o->texture.openglContextId) {
		//dd_log("font needs refresh");
		o->openglContextId = o->texture.openglContextId;
		avdl_font_releaseAllGlyphs(o);
		return 1;
	}
	#endif

	return 0;
}

void avdl_font_addCustomIcon(struct avdl_font *o, const char *keyword, struct dd_image *image) {
	#if !defined( AVDL_DIRECT3D11 )
	if (o->customIconCount >= 10) {
		dd_log("avdl error: too many custom icons in font");
		return;
	}
	o->customIcon[o->customIconCount] = image;
	o->customIconKeyword[o->customIconCount] = keyword;
	o->customIconCount++;
	#endif
}
