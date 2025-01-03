#include "dd_vec4.h"
#include "dd_math.h"
#include "avdl_log.h"

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
		#if defined(AVDL_QUEST2)
		// Quest 2 only
                new_vec.cell[x] =
                        (o->cell[0] *m->cell[(x *4) +0]) +
                        (o->cell[1] *m->cell[(x *4) +1]) +
                        (o->cell[2] *m->cell[(x *4) +2]) +
                        (o->cell[3] *m->cell[(x *4) +3]);
		#else
                new_vec.cell[x] =
                        (o->cell[0] *m->cell[(x %4) +0]) +
                        (o->cell[1] *m->cell[(x %4) +4]) +
                        (o->cell[2] *m->cell[(x %4) +8]) +
                        (o->cell[3] *m->cell[(x %4) +12]);
		#endif
        }

        for (x = 0; x < 4; x++) {
                o->cell[x] = new_vec.cell[x];
        }
}

float dd_vec4_dot(struct dd_vec4 *a, struct dd_vec4 *b) {
	return a->cell[0] *b->cell[0]
		+a->cell[1] *b->cell[1]
		+a->cell[2] *b->cell[2]
		+a->cell[3] *b->cell[3];
}

void dd_vec4_cross(struct dd_vec4 *a, struct dd_vec4 *b) {
	dd_vec4_set(a,
		dd_vec4_getY(a) *dd_vec4_getZ(b) -dd_vec4_getZ(a) *dd_vec4_getY(b),
		dd_vec4_getZ(a) *dd_vec4_getX(b) -dd_vec4_getX(a) *dd_vec4_getZ(b),
		dd_vec4_getX(a) *dd_vec4_getY(b) -dd_vec4_getY(a) *dd_vec4_getX(b),
		dd_vec4_getW(a)
	);
}

void dd_vec4_print(struct dd_vec4 *o) {
	avdl_log("dd_vec4: %f %f %f %f",
		o->cell[0],
		o->cell[1],
		o->cell[2],
		o->cell[3]
	);
}

void dd_vec4_normalise(struct dd_vec4 *o) {
	float magn = dd_vec3_magnitude(o);
	o->cell[0] /= magn;
	o->cell[1] /= magn;
	o->cell[2] /= magn;
	o->cell[3] /= magn;
}

float dd_vec4_magnitude(struct dd_vec4 *o) {
	return dd_math_sqrt(dd_math_pow(o->cell[0], 2) +dd_math_pow(o->cell[1], 2) +dd_math_pow(o->cell[2], 2) +dd_math_pow(o->cell[3], 2));
}
