#include "avdl_quaternion.h"
#include "shared/avdl_math.h"
#include "shared/avdl_log.h"
#include "dd_matrix.h"
#include "avdl_vec3.h"

void avdl_quaternion_create(struct avdl_quaternion *o) {
	avdl_quaternion_Identity(o);
}

void avdl_quaternion_Identity(struct avdl_quaternion *o) {
	o->x = 0;
	o->y = 0;
	o->z = 0;
	o->w = 1;
}

void avdl_quaternion_RotationFromEuler(struct avdl_quaternion *o, float x, float y, float z) {
	float rotX = dd_math_dec2rad(x) /2;
	float rotY = dd_math_dec2rad(y) /2;
	float rotZ = dd_math_dec2rad(z) /2;
	o->x = sin(rotX)*cos(rotY)*cos(rotZ) - cos(rotX)*sin(rotY)*sin(rotZ);
	o->y = cos(rotX)*sin(rotY)*cos(rotZ) + sin(rotX)*cos(rotY)*sin(rotZ);
	o->z = cos(rotX)*cos(rotY)*sin(rotZ) - sin(rotX)*sin(rotY)*cos(rotZ);
	o->w = cos(rotX)*cos(rotY)*cos(rotZ) + sin(rotX)*sin(rotY)*sin(rotZ);
}

void avdl_quaternion_Multiply(struct avdl_quaternion *o, struct avdl_quaternion *q) {
	float x = o->w*q->x + o->x*q->w + o->y*q->z - o->z*q->y;
	float y = o->w*q->y - o->x*q->z + o->y*q->w + o->z*q->x;
	float z = o->w*q->z + o->x*q->y - o->y*q->x + o->z*q->w;
	float w = o->w*q->w - o->x*q->x - o->y*q->y - o->z*q->z;

	o->x = x;
	o->y = y;
	o->z = z;
	o->w = w;
}

void avdl_quaternion_ToRotationMatrix(struct avdl_quaternion *o, struct dd_matrix *m) {

	float xx = o->x * o->x;
	float yy = o->y * o->y;
	float zz = o->z * o->z;
	float xy = o->x * o->y;
	float xz = o->x * o->z;
	float yz = o->y * o->z;
	float wx = o->w * o->x;
	float wy = o->w * o->y;
	float wz = o->w * o->z;

	m->cell[0] = 1 - (2 * ((dd_math_pow(yy, 2) + dd_math_pow(zz, 2))));
	m->cell[1] = 2 * (xy -wz);
	m->cell[2] = 2 * (xz +wy);
	m->cell[3] = 0;

	m->cell[4] = 2 * (xy +wz);
	m->cell[5] = 1 - ( 2 * (dd_math_pow(xx, 2) +dd_math_pow(zz, 2)));
	m->cell[6] = 2 * (yz -wx);
	m->cell[7] = 0;

	m->cell[8] = 2 * (xz -wy);
	m->cell[9] = 2 * (yz -wx);
	m->cell[10] = 1 - ( 2 * (dd_math_pow(xx, 2) +dd_math_pow(yy, 2)) );
	m->cell[11] = 0;

	m->cell[12] = 0;
	m->cell[13] = 0;
	m->cell[14] = 0;
	m->cell[15] = 1;
}

void avdl_quaternion_Print(struct avdl_quaternion *o) {
	avdl_log("Quaternion: %f %f %f %f", o->x, o->y, o->z, o->w);
}

void avdl_quaternion_ToEuler(struct avdl_quaternion *o, struct avdl_vec3 *v) {
	float x = dd_math_atan2( 2 *((o->w *o->x) +(o->y *o->z)), 1 - (2 * (dd_math_pow(o->x, 2) + dd_math_pow(o->y, 2))));
	float y = dd_math_asin ( 2 *((o->w *o->y) - (o->z * o->x)));
	float z = dd_math_atan2( 2 *((o->w *o->z) +(o->x *o->y)), 1 - (2 * (dd_math_pow(o->y, 2) + dd_math_pow(o->z, 2))));

	// roll (x-axis rotation)
	float sinr_cosp = 2.0 * (o->w * o->x + o->y * o->z);
	float cosr_cosp = 1.0 - 2.0 * (o->x * o->x + o->y * o->y);
	x = dd_math_atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float sinp = dd_math_sqrt(1 + 2 * (o->w * o->y - o->x * o->z));
	float cosp = dd_math_sqrt(1 - 2 * (o->w * o->y - o->x * o->z));
	y = 2.0 * dd_math_atan2(sinp, cosp) -3.14 / 2.0;

	// yaw (z-axis rotation)
	float siny_cosp = 2 * (o->w * o->z + o->x * o->y);
	float cosy_cosp = 1 - 2 * (o->y * o->y + o->z * o->z);
	z = dd_math_atan2(siny_cosp, cosy_cosp);

	x = dd_math_rad2dec(x);
	y = dd_math_rad2dec(y);
	z = dd_math_rad2dec(z);

	v->x = x;
	v->y = y;
	v->z = z;
}

void avdl_quaternion_Copy(struct avdl_quaternion *o, struct avdl_quaternion *q) {
	o->x = q->x;
	o->y = q->y;
	o->z = q->z;
	o->w = q->w;
}

void avdl_quaternion_Conjugate(struct avdl_quaternion *o) {
	o->x = -o->x;
	o->y = -o->y;
	o->z = -o->z;
}
