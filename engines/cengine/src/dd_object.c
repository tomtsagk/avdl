#include "dd_object.h"
#include "dd_opengl.h"

void dd_object_create(struct dd_object *o) {
/*
	dd_mesh_create((struct dd_mesh *)o);

	o->x = 0;
	o->y = 0;
	o->z = 0;
	*/
}

void dd_object_clean(struct dd_object *o) {
	//dd_mesh_clean((struct dd_mesh *)o);
}

void dd_object_draw(struct dd_object *o) {
	/*

	glPushMatrix();
	glTranslatef(o->x, o->y, o->z);

	dd_mesh_draw((struct dd_mesh *)o);

	glPopMatrix();
	*/
}

void dd_object_setX(struct dd_object *o, float val) {
	//o->x = val;
}

void dd_object_setY(struct dd_object *o, float val) {
	//o->y = val;
}

void dd_object_setZ(struct dd_object *o, float val) {
	//o->z = val;
}

void dd_object_setPosition(struct dd_object *o, float x, float y, float z) {
/*
	dd_object_setX(o, x);
	dd_object_setY(o, y);
	dd_object_setZ(o, z);
	*/
}
