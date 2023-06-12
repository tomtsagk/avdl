#ifndef DD_TEXT_H
#define DD_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "avdl_cengine.h"
#include "dd_image.h"

void dd_string3d_activate(const char *src, int src_type, float fColumns, float fRows, float fWidth, float fHeight);
int dd_string3d_isActive();

void dd_string3d_init();
void dd_string3d_deinit();

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
	int width;
};

struct dd_string3d {

	struct dd_dynamic_array textMeshes;

	// Align
	enum dd_string3d_align align;
	enum dd_string3d_align_vertical alignv;

	float colorFront[3];
	float colorBack[3];

	int len;

	void (*setText)(struct dd_string3d *, const char *text);

	void (*setAlign)(struct dd_string3d *, enum dd_string3d_align);
	void (*setAlignVertical)(struct dd_string3d *, enum dd_string3d_align_vertical);
	void (*draw)(struct dd_string3d *);
	void (*drawInt)(struct dd_string3d *, int num);
	void (*drawLimit)(struct dd_string3d *, int limit);

	void (*clean)(struct dd_string3d *);
};

void dd_string3d_create(struct dd_string3d *o);
void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al);
void dd_string3d_setAlignVertical(struct dd_string3d *o, enum dd_string3d_align_vertical al);

void dd_string3d_draw(struct dd_string3d *o);
void dd_string3d_drawInt(struct dd_string3d *o, int num);
void dd_string3d_drawLimit(struct dd_string3d *o, int limit);

void dd_string3d_clean(struct dd_string3d *o);

void dd_string3d_setText(struct dd_string3d *o, const char *text);

void dd_string3d_setKerning(float nkerning);

#ifdef __cplusplus
}
#endif

#endif
