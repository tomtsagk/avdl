#include "avdl_font.h"
#include "dd_math.h"
#include "dd_log.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

FT_Library library;

#define FONT_ATLAS_WIDTH 1024
#define FONT_ATLAS_HEIGHT 1024

//#define FONT_GLYPH_SIZE 100
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

	if (library) {
		FT_Done_FreeType( library );
	}
	return 0;
}

void avdl_font_create(struct avdl_font *o) {

	// functions
	o->set = avdl_font_set;

	for (int i = 0; i < 100; i++) {
		o->glyphs[i].uses = 0;
	}

	dd_image_create(&o->texture);
	o->texture.width = FONT_ATLAS_WIDTH;
	o->texture.height = FONT_ATLAS_HEIGHT;
	o->texture.pixelFormat = GL_RGBA;
	o->texture.openglContextId = avdl_graphics_getContextId();
	o->outline_thickness = 0;
	o->fontData = 0;

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

	o->face = 0;

}

void avdl_font_clean(struct avdl_font *o) {
	dd_image_clean(&o->texture);
}

int avdl_font_registerGlyph(struct avdl_font *o, int unicode_hex) {

	if (!o->face) {
		return -1;
	}

	// check if glyph exists
	for (int i = 0; i < 100; i++) {
		if (o->glyphs[i].uses > 0 && unicode_hex == o->glyphs[i].id) {
			o->glyphs[i].uses++;
			return i;
		}
	}

	// find a free slot
	int glyph_id = -1;
	for (int i = 0; i < 100; i++) {
		if (o->glyphs[i].uses <= 0) {
			glyph_id = i;
			break;
		}
	}
	if (glyph_id == -1) {
		dd_log("cannot make more glyphs for this font");
		return -1;
	}

	FT_Error error = FT_Set_Pixel_Sizes(
		o->face,   /* handle to face object */
		0,      /* pixel_width           */
		FONT_GLYPH_SIZE      /* pixel_height          */
	);
	if (error) {
		dd_log("error set pixel sizes");
	}

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	// clear cell
	for (int x = 0; x < FONT_GLYPH_SIZE; x++)
	for (int y = 0; y < FONT_GLYPH_SIZE; y++) {
		int ry = FONT_GLYPH_SIZE-1 -y;
		int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%10)*FONT_GLYPH_SIZE*4 +((glyph_id/10)*o->texture.width*4*FONT_GLYPH_SIZE);
		o->texture.pixelsb[index+0] = 255;
		o->texture.pixelsb[index+1] = 255;
		o->texture.pixelsb[index+2] = 255;
		o->texture.pixelsb[index+3] = 0;
	}
	#else
	// clear cell
	for (int x = 0; x < FONT_GLYPH_SIZE; x++)
	for (int y = 0; y < FONT_GLYPH_SIZE; y++) {
		int ry = FONT_GLYPH_SIZE-1 -y;
		int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%10)*FONT_GLYPH_SIZE*4 +((glyph_id/10)*o->texture.width*4*FONT_GLYPH_SIZE);
		o->texture.pixels[index+0] = 1.0;
		o->texture.pixels[index+1] = 1.0;
		o->texture.pixels[index+2] = 1.0;
		o->texture.pixels[index+3] = 0.0;
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

		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		// render glyph on texture
		for (int x = 0; x < bitmap_stroke->width; x++)
		for (int y = 0; y < bitmap_stroke->rows ; y++) {
			int ry = bitmap_stroke->rows-1 -y;
			int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%10)*FONT_GLYPH_SIZE*4 +((glyph_id/10)*o->texture.width*4*FONT_GLYPH_SIZE);
			o->texture.pixelsb[index+0] = 0;
			o->texture.pixelsb[index+1] = 0;
			o->texture.pixelsb[index+2] = 0;
			o->texture.pixelsb[index+3] = dd_math_min(bitmap_stroke->buffer[y*bitmap_stroke->width +x] *255, 255);
		}
		#else
		// render glyph on texture
		for (int x = 0; x < bitmap_stroke->width; x++)
		for (int y = 0; y < bitmap_stroke->rows ; y++) {
			int ry = bitmap_stroke->rows-1 -y;
			int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%10)*FONT_GLYPH_SIZE*4 +((glyph_id/10)*o->texture.width*4*FONT_GLYPH_SIZE);
			o->texture.pixels[index+0] = 0.0;
			o->texture.pixels[index+1] = 0.0;
			o->texture.pixels[index+2] = 0.0;
			o->texture.pixels[index+3] = bitmap_stroke->buffer[y*bitmap_stroke->width +x];
		}
		#endif
		cx = bitmap_stroke->width;
		cy = bitmap_stroke->rows;
	}

	error = FT_Load_Char( o->face, unicode_hex, FT_LOAD_RENDER );
	if (error) {
		dd_log("avdl: error loading char glyph: %d", unicode_hex);
	}

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

	int cx_fill  = o->face->glyph->bitmap.width;
	int cy_fill  = o->face->glyph->bitmap.rows;
	int offset_x = (cx - cx_fill) / 2; // offset because the bitmap my be smaller, 
	int offset_y = (cy - cy_fill) / 2; // then the former

	// render glyph on texture
	for (int x = 0; x < o->face->glyph->bitmap.width; x++)
	for (int y = 0; y < o->face->glyph->bitmap.rows ; y++) {
		int ry = o->face->glyph->bitmap.rows-1 -y;
		if (o->outline_thickness > 0) {
			ry += offset_y;
		}
		int index = (ry*o->texture.width*4) +x*4+0 +(glyph_id%10)*FONT_GLYPH_SIZE*4 +((glyph_id/10)*o->texture.width*4*FONT_GLYPH_SIZE);
		if (o->outline_thickness > 0) {
			index += offset_x *4;
		}
		float alpha = o->face->glyph->bitmap.buffer[y*o->face->glyph->bitmap.width +x];
		/*
		o->texture.pixels[index+0] = 1.0;
		o->texture.pixels[index+1] = 1.0;
		o->texture.pixels[index+2] = 1.0;
		o->texture.pixels[index+3] = alpha;
		*/
		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		o->texture.pixelsb[index+0] = dd_math_min(255, o->texture.pixelsb[index+0] +255 *alpha);
		o->texture.pixelsb[index+1] = dd_math_min(255, o->texture.pixelsb[index+1] +255 *alpha);
		o->texture.pixelsb[index+2] = dd_math_min(255, o->texture.pixelsb[index+2] +255 *alpha);
		o->texture.pixelsb[index+3] = dd_math_max(o->texture.pixelsb[index+3], dd_math_min(alpha *255, 255));
		#else
		o->texture.pixels[index+0] += 1.0 *alpha;
		o->texture.pixels[index+1] += 1.0 *alpha;
		o->texture.pixels[index+2] += 1.0 *alpha;
		o->texture.pixels[index+3] = dd_math_max(o->texture.pixels[index+3], alpha);
		#endif
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

	return glyph_id;
}

int avdl_font_releaseGlyph(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return -1;
	}
	o->glyphs[glyph_id].uses--;
	return 0;
}

float avdl_font_getTexCoordX(struct avdl_font *o, int glyph_id) {
	return ((glyph_id%10)*FONT_GLYPH_SIZE) /(float) FONT_ATLAS_WIDTH;
}

float avdl_font_getTexCoordY(struct avdl_font *o, int glyph_id) {
	return ((glyph_id/10)*FONT_GLYPH_SIZE) /(float) FONT_ATLAS_HEIGHT;
}

float avdl_font_getTexCoordW(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].texcoordW;
}

float avdl_font_getTexCoordH(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].texcoordH;
}

float avdl_font_getGlyphWidth(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].width;
}

float avdl_font_getGlyphHeight(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].height;
}

float avdl_font_getGlyphLeft(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].left;
}

float avdl_font_getGlyphTop(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].top;
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

	o->fontData = 0;

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	AAsset* fontFile = AAssetManager_open(aassetManager, name, AASSET_MODE_UNKNOWN);
	if (!fontFile) {
		dd_log("unable to open font file");
		return;
	}
	off_t fontDataSize = AAsset_getLength(fontFile);

	o->fontData = malloc(sizeof(FT_Byte) *fontDataSize);
	if (!o->fontData) {
		dd_log("error during malloc");
		return;
	}
	AAsset_read(fontFile, o->fontData, fontDataSize);
	AAsset_close(fontFile);

	if (FT_New_Memory_Face(library, o->fontData, fontDataSize, 0, &o->face)) {
		dd_log("Load memory failed");
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

}

float avdl_font_getGlyphAdvance(struct avdl_font *o, int glyph_id) {
	if (glyph_id < 0 || glyph_id >= 100) {
		return 0;
	}
	return o->glyphs[glyph_id].advance;
}
