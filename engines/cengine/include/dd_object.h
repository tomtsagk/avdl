#ifndef DD_OBJECT_H
#define DD_OBJECT_H

#include "dd_mesh.h"

/* object is a subclass of mesh
 * it is essentially a mesh with a position
 */
struct dd_object {
	struct dd_mesh p;

	float x, y, z;
};

// constructor - destructor
void dd_object_create(struct dd_object *);
void dd_object_clean(struct dd_object *);

// draw
void dd_object_draw(struct dd_object *);

// set position
void dd_object_setX(struct dd_object *, float);
void dd_object_setY(struct dd_object *, float);
void dd_object_setZ(struct dd_object *, float);
void dd_object_setPosition(struct dd_object *, float, float, float);

#endif
