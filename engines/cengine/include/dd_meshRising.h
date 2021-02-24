#ifndef DD_MESHRISING_H
#define DD_MESHRISING_H

#include "dd_meshColour.h"
#include "dd_opengl.h"

struct dd_meshRising {
	struct dd_meshColour parent;
	float animationCurrent;
	float animationMax;
	GLuint animationCurrentLocation;
	GLuint animationMaxLocation;
	void (*set_animation_max)(struct dd_mesh *, float);
	void (*set_animation_current)(struct dd_mesh *, float);
};

// constructor
void dd_meshRising_create(struct dd_meshRising *);

/* Free and Draw functions */
void dd_meshRising_draw(struct dd_meshRising *m);

void dd_meshRising_set_animation_max(struct dd_meshRising *m, float);
void dd_meshRising_set_animation_current(struct dd_meshRising *m, float);

#endif /* MESHCOLOUR_H */
