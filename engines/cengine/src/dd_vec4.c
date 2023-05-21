#include "dd_vec4.h"

void dd_vec4_create(struct dd_vec4 *o) {
	o->cell[0] = 0;
	o->cell[1] = 0;
	o->cell[2] = 0;
	o->cell[3] = 0;
	o->set = dd_vec4_set;
	o->getX = dd_vec4_getX;
	o->getY = dd_vec4_getY;
	o->getZ = dd_vec4_getZ;
	o->getW = dd_vec4_getW;
}

void dd_vec4_set(struct dd_vec4 *o, float x, float y, float z, float w) {
	o->cell[0] = x;
	o->cell[1] = y;
	o->cell[2] = z;
	o->cell[3] = w;
}

float dd_vec4_getX(struct dd_vec4 *o) {
	return o->cell[0];
}
float dd_vec4_getY(struct dd_vec4 *o) {
	return o->cell[1];
}
float dd_vec4_getZ(struct dd_vec4 *o) {
	return o->cell[2];
}
float dd_vec4_getW(struct dd_vec4 *o) {
	return o->cell[3];
}

void dd_vec4_add(struct dd_vec4 *o1, float x, float y, float z, float w) {
	o1->cell[0] += x;
	o1->cell[1] += y;
	o1->cell[2] += z;
	o1->cell[3] += w;
}

void dd_vec4_addVec3(struct dd_vec4 *o1, struct dd_vec4 *o2) {
	o1->cell[0] += o2->cell[0];
	o1->cell[1] += o2->cell[1];
	o1->cell[2] += o2->cell[2];
	o1->cell[3] += o2->cell[3];
}

void dd_vec4_clean(struct dd_vec4 *o) {
}

void dd_vec4_multiply(struct dd_vec4 *o, struct dd_matrix *m) {
        struct dd_vec4 new_vec;
        int x;
        for (x = 0; x < 4; x++) {
		// Quest 2 only ?
                new_vec.cell[x] =
                        (o->cell[0] *m->cell[(x *4) +0]) +
                        (o->cell[1] *m->cell[(x *4) +1]) +
                        (o->cell[2] *m->cell[(x *4) +2]) +
                        (o->cell[3] *m->cell[(x *4) +3]);
        }

        for (x = 0; x < 4; x++) {
                o->cell[x] = new_vec.cell[x];
        }
}
