#ifndef DD_TEXT_H
#define DD_TEXT_H

void dd_string3d_activate(const char *fontname);
int dd_string3d_isActive();

void dd_string3d_init();

enum dd_string3d_align {
	DD_STRING3D_ALIGN_LEFT,
	DD_STRING3D_ALIGN_CENTER,
	DD_STRING3D_ALIGN_RIGHT,
};

struct dd_string3d {
	// Align
	enum dd_string3d_align align;

	float colorFront[3];
	float colorBack[3];

	void (*setAlign)(struct dd_string3d *, enum dd_string3d_align);
	void (*setColorFront)(struct dd_string3d *, float r, float g, float b);
	void (*setColorBack)(struct dd_string3d *, float r, float g, float b);
	void (*draw)(struct dd_string3d *, const char *text);
	void (*drawInt)(struct dd_string3d *, int num);

	void (*clean)(struct dd_string3d *);
};

void dd_string3d_create(struct dd_string3d *o);
void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al);

void dd_string3d_setColorFront(struct dd_string3d *, float r, float g, float b);
void dd_string3d_setColorBack(struct dd_string3d *, float r, float g, float b);

void dd_string3d_draw(struct dd_string3d *o, const char* text);
void dd_string3d_drawInt(struct dd_string3d *o, int num);

void dd_string3d_clean(struct dd_string3d *o);

#endif
