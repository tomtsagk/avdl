#include "dd_matrix.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "dd_dynamic_array.h"
#include "dd_log.h"

void dd_matrix_create(float mat[16]) {
}

/* init matrix to identity */
void dd_matrix_identity(float mat[16]) {
	/* make only the middle line 1's, the rest 0's */
	int i;
	for (i = 0; i < 16; i++) {
		if (i % 5 == 0) {
			mat[i] = 1.0f;
		}
		else {
			mat[i] = 0.0f;
		}
	}
}

/* init matrix to identity, except translation 
 * something is wrong here, commented out for now
 */
void dd_matrix_identityt(float mat[16]) {
	/* this counts until 12, so it skips translation
	 * which is on position 12, 13, 14
	 	(at least on opengl's inverted method)
	 */
	int i;
	for (i = 0; i < 12; i++) {
		if (i % 5 == 0) { mat[i] = 1.0f; } 
		else 		{ mat[i] = 0.0f; }
	}

	/* let's not forget the last element */
	mat[15] = 1.0f;
}

/* Translate - Add */
void dd_matrix_translatea(float mat[16], float x, float y, float z) {
	mat[12] += x;
	mat[13] += y;
	mat[14] += z;
}

/* Translate - Set */
void dd_matrix_translates(float mat[16], float x, float y, float z) {
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;
}

/* Translate - Multiply */
void dd_matrix_translatem(float mat[16], float x, float y, float z) {

	/* Create new matrix, set translation, multiply with given matrix */
	float m2[16];
	dd_matrix_identity(m2);
	dd_matrix_translates(m2, x, y, z);
	dd_matrix_mult(mat, m2);
	/*
	struct dd_matrix m;
	dd_matrix_identity(m.mat);
	dd_matrix_translates(&m, x, y, z);
	dd_matrix_mult(mat->mat, m.mat);
	*/
}

/* Scale - Add */
void dd_matrix_scalea(float mat[16], float x, float y, float z) {
	mat[ 0] += x;
	mat[ 5] += y;
	mat[10] += z;
}

/* Scale - Set */
void dd_matrix_scales(float mat[16], float x, float y, float z) {
	mat[ 0] = x;
	mat[ 5] = y;
	mat[10] = z;
}

void dd_matrix_scalem(float mat[16], float x, float y, float z) {
	float m[16];
	dd_matrix_identity(m);
	dd_matrix_scales(m, x, y, z);
	dd_matrix_mult(mat, m);
}

/* Rotate 
 * these are highly experimental function, as I'm
 * still trying to wrap my head around 3d rotation
 */
void dd_matrix_rotate_x(float mat[16], float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			1, 0, 0, 0,
			0, cos(rad), sin(rad), 0,
			0, -sin(rad),  cos(rad), 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(mat, rot_mat);
}

void dd_matrix_rotate_y(float mat[16], float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			cos(rad), 0, -sin(rad), 0,
			0, 1, 0, 0,
			sin(rad), 0, cos(rad), 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(mat, rot_mat);
}

void dd_matrix_rotate_z(float mat[16], float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			cos(rad), sin(rad), 0, 0,
			-sin(rad), cos(rad), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(mat, rot_mat);
}

/* Removes translation, rotates, and re-applies translation 
 * I don't remember why I did that, by let's keep it for now
 */
void dd_matrix_rotatelocal_x(float mat[16], float rad) {
	float x, y, z;
	x = dd_matrix_x(mat);
	y = dd_matrix_y(mat);
	z = dd_matrix_z(mat);
	dd_matrix_rotate_x(mat, rad);
	dd_matrix_translates(mat, x, y, z);
}

void dd_matrix_rotatelocal_y(float mat[16], float rad) {
	float x, y, z;
	x = dd_matrix_x(mat);
	y = dd_matrix_y(mat);
	z = dd_matrix_z(mat);
	dd_matrix_translates(mat, 0.0f, 0.0f, 0.0f);
	dd_matrix_rotate_y(mat, rad);
	dd_matrix_translates(mat, x, y, z);
}

void dd_matrix_rotatelocal_z(float mat[16], float rad) {
	float x, y, z;
	x = dd_matrix_x(mat);
	y = dd_matrix_y(mat);
	z = dd_matrix_z(mat);
	dd_matrix_rotate_z(mat, rad);
	dd_matrix_translates(mat, x, y, z);
}

void dd_matrix_rotatem(float mat[16], float angle, float x, float y, float z) {
	float m[16];
	float angleRadians = 3.14/180.0 *(-angle);
	m[0]  = pow(x, 2) *(1-cos(angleRadians))+cos(angleRadians);
	m[1]  = x*y*(1-cos(angleRadians))-z*sin(angleRadians);
	m[2]  = x*z*(1-cos(angleRadians))+y*sin(angleRadians);
	m[3]  = 0;
	m[4]  = y*x*(1-cos(angleRadians))+z*sin(angleRadians);
	m[5]  = pow(y, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m[6]  = y*z*(1-cos(angleRadians))-x*sin(angleRadians);
	m[7]  = 0;
	m[8]  = x*z*(1-cos(angleRadians))-y*sin(angleRadians);
	m[9]  = y*z*(1-cos(angleRadians))+x*sin(angleRadians);
	m[10] = pow(z, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m[11] = 0;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
	dd_matrix_mult(mat, m);
}
/* Matrix multiplication */
void dd_matrix_mult(float mat1[16], float mat2[16]) {
	float new_mat[16];
	int x;
	for (x = 0; x < 16; x++) {
		new_mat[x] =
			(mat1[(x%4)+0] *mat2[(x/4)*4+0]) +
			(mat1[(x%4)+4] *mat2[(x/4)*4+1]) +
			(mat1[(x%4)+8] *mat2[(x/4)*4+2]) +
			(mat1[(x%4)+12] *mat2[(x/4)*4+3]);
	}

	for (x = 0; x < 16; x++) {
		mat1[x] = new_mat[x];
	}
}

void dd_matrix_copy(float mat1[16], float mat2[16]) {
	for (int i = 0; i < 16; i++) {
		mat1[i] = mat2[i];
	}
}

void dd_matrix_copy_rs(float mat1[16], float mat2[16]) {
	for (int i = 0; i < 12; i++) {
		mat1[i] = mat2[i];
	}
	mat1[15] = mat2[15];
}

void dd_matrix_approach(float mat[16], float target[16], float counter) {
	for (int i = 0; i < 16; i++) {
		mat[i] = mat[i] +(target[i] -mat[i]) *counter;
	}
}

void dd_matrix_approach_rs(float mat[16], float target[16], float counter) {
	for (int i = 0; i < 12; i++) {
		mat[i] = mat[i] +(target[i] -mat[i]) *counter;
	}
	mat[15] = mat[15] +(target[15] -mat[15]) *counter;
}

/* Getters */
float dd_matrix_x(float mat[16]) { return mat[12]; }
float dd_matrix_y(float mat[16]) { return mat[13]; }
float dd_matrix_z(float mat[16]) { return mat[14]; }

/* Print the matrix, this is clearly meant for debugging only,
	don't use it in production code, seriously
 */
void dd_matrix_print(float mat[16]) {
	int i;
	for (i = 0; i < 16; i += 4) {
		dd_log("%f %f %f %f",
			mat[(i *4) +0],
			mat[(i *4) +1],
			mat[(i *4) +2],
			mat[(i *4) +3]
		);

		if ((i -3) % 4 == 0) {
			dd_log("");
		}
	}
}

/*
 * custom matrix stack implementation
 */
struct dd_matrix dd_cam[DD_MATRIX_STACK_LIMIT];
int dd_cam_index = 0;

void dd_matrix_globalInit() {
	dd_cam_index = 0;
	dd_matrix_identity((float *) &dd_cam[0]);
}

void dd_matrix_push() {
	if (dd_cam_index < (DD_MATRIX_STACK_LIMIT -1)) {
		dd_cam_index++;
		dd_matrix_copy((float *)&dd_cam[dd_cam_index], (float *)&dd_cam[dd_cam_index-1]);
	}
	else {
		dd_log("dd_matrix: maximum matrix stack reached");
		exit(-1);
	}
}

void dd_matrix_pop() {
	if (dd_cam_index > 0) {
		dd_cam_index--;
	}
	else {
		dd_log("dd_matrix: no matrix to pop");
		exit(-1);
	}
}

struct dd_matrix *dd_matrix_globalGet() {
	return &dd_cam[dd_cam_index];
}
