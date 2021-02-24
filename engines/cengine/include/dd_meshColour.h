#ifndef DD_MESHCOLOUR_H
#define DD_MESHCOLOUR_H

#include "dd_mesh.h"

struct dd_meshColour {
	struct dd_mesh parent;
	int dirtyColours;
	float *c;
	void (*set_colour)(struct dd_mesh *m, float r, float g, float b);
};

// constructor
void dd_meshColour_create(struct dd_meshColour *);

/* Free and Draw functions */
void dd_meshColour_clean(struct dd_meshColour *m);
void dd_meshColour_draw(struct dd_meshColour *m);

// functions to give the mesh its shape
void dd_meshColour_set_primitive(struct dd_meshColour *m, enum dd_primitives shape);
void dd_meshColour_load(struct dd_meshColour *m, const char *filename);

void dd_meshColour_copy(struct dd_meshColour *dest, struct dd_meshColour *src);

void dd_meshColour_set_colour(struct dd_meshColour *m, float r, float g, float b);

#endif /* MESHCOLOUR_H */
