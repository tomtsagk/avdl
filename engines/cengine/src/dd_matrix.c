#include "dd_matrix.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "shared/avdl_log.h"
#include <string.h>
#include "avdl_vec3.h"

void dd_matrix_create(struct dd_matrix *m) {
	dd_matrix_identity(m);
}
void dd_matrix_clean(struct dd_matrix *m) {}

/* init matrix to identity */
void dd_matrix_identity(struct dd_matrix *m) {
	/* make only the middle line 1's, the rest 0's */
	int i;
	for (i = 0; i < 16; i++) {
		if (i % 5 == 0) {
			m->cell[i] = 1.0f;
		}
		else {
			m->cell[i] = 0.0f;
		}
	}
}

/* init matrix to identity, except translation 
 * something is wrong here, commented out for now
 */
void dd_matrix_identityt(struct dd_matrix *m) {
	/* this counts until 12, so it skips translation
	 * which is on position 12, 13, 14
	 	(at least on opengl's inverted method)
	 */
	int i;
	for (i = 0; i < 12; i++) {
		if (i % 5 == 0) { m->cell[i] = 1.0f; } 
		else 		{ m->cell[i] = 0.0f; }
	}

	/* let's not forget the last element */
	m->cell[15] = 1.0f;
}

/* Translate - Add */
void dd_matrix_translatea(struct dd_matrix *m, float x, float y, float z) {
	m->cell[12] += x;
	m->cell[13] += y;
	m->cell[14] += z;
}

/* Translate - Set */
void dd_matrix_translates(struct dd_matrix *m, float x, float y, float z) {
	// the quest 2 engine is using different ways to organise
	// matrices, this is a temp solution
	#if defined( AVDL_DIRECT3D11 ) || defined( AVDL_QUEST2 )
	m->cell[ 3] = x;
	m->cell[ 7] = y;
	m->cell[11] = z;
	#else
	m->cell[12] = x;
	m->cell[13] = y;
	m->cell[14] = z;
	#endif
}

/* Translate - Multiply */
void dd_matrix_translate(struct dd_matrix *m, float x, float y, float z) {

	/* Create new matrix, set translation, multiply with given matrix */
	struct dd_matrix m2;
	dd_matrix_identity(&m2);
	dd_matrix_translates(&m2, x, y, z);
	dd_matrix_mult(m, &m2);
}

/* Scale - Add */
void dd_matrix_scalea(struct dd_matrix *m, float x, float y, float z) {
	m->cell[ 0] += x;
	m->cell[ 5] += y;
	m->cell[10] += z;
}

/* Scale - Set */
void dd_matrix_scales(struct dd_matrix *m, float x, float y, float z) {
	m->cell[ 0] = x;
	m->cell[ 5] = y;
	m->cell[10] = z;
}

void dd_matrix_scale(struct dd_matrix *m, float x, float y, float z) {
	struct dd_matrix m2;
	dd_matrix_identity(&m2);
	dd_matrix_scales(&m2, x, y, z);
	dd_matrix_mult(m, &m2);
}

/* Rotate 
 * these are highly experimental function, as I'm
 * still trying to wrap my head around 3d rotation
 */
void dd_matrix_rotate_x(struct dd_matrix *m, float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			1, 0, 0, 0,
			0, cos(rad), sin(rad), 0,
			0, -sin(rad),  cos(rad), 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(m, (struct dd_matrix *)&rot_mat);
}

void dd_matrix_rotate_y(struct dd_matrix *m, float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			cos(rad), 0, -sin(rad), 0,
			0, 1, 0, 0,
			sin(rad), 0, cos(rad), 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(m, (struct dd_matrix *)&rot_mat);
}

void dd_matrix_rotate_z(struct dd_matrix *m, float rad) {
	/* Create rotation matrix */
	float rot_mat[] = {
			cos(rad), sin(rad), 0, 0,
			-sin(rad), cos(rad), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};

	/* Multiply given matrix with rotation matrix */
	dd_matrix_mult(m, (struct dd_matrix *)&rot_mat);
}

/* Removes translation, rotates, and re-applies translation 
 * I don't remember why I did that, by let's keep it for now
 */
void dd_matrix_rotatelocal_x(struct dd_matrix *m, float rad) {
	float x, y, z;
	x = dd_matrix_x(m);
	y = dd_matrix_y(m);
	z = dd_matrix_z(m);
	dd_matrix_rotate_x(m, rad);
	dd_matrix_translates(m, x, y, z);
}

void dd_matrix_rotatelocal_y(struct dd_matrix *m, float rad) {
	float x, y, z;
	x = dd_matrix_x(m);
	y = dd_matrix_y(m);
	z = dd_matrix_z(m);
	dd_matrix_translates(m, 0.0f, 0.0f, 0.0f);
	dd_matrix_rotate_y(m, rad);
	dd_matrix_translates(m, x, y, z);
}

void dd_matrix_rotatelocal_z(struct dd_matrix *m, float rad) {
	float x, y, z;
	x = dd_matrix_x(m);
	y = dd_matrix_y(m);
	z = dd_matrix_z(m);
	dd_matrix_rotate_z(m, rad);
	dd_matrix_translates(m, x, y, z);
}

void dd_matrix_rotate(struct dd_matrix *m, float angle, float x, float y, float z) {
	struct dd_matrix m2;
	float angleRadians = 3.14/180.0 *(-angle);
	#if defined( AVDL_DIRECT3D11 ) || defined( AVDL_QUEST2 )
	m2.cell[ 0] = pow(x, 2) *(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[ 1] = y*x*(1-cos(angleRadians))+z*sin(angleRadians);
	m2.cell[ 2] = x*z*(1-cos(angleRadians))-y*sin(angleRadians);
	m2.cell[ 3] = 0;
	m2.cell[ 4] = x*y*(1-cos(angleRadians))-z*sin(angleRadians);
	m2.cell[ 5] = pow(y, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[ 6] = y*z*(1-cos(angleRadians))+x*sin(angleRadians);
	m2.cell[ 7] = 0;
	m2.cell[ 8] = x*z*(1-cos(angleRadians))+y*sin(angleRadians);
	m2.cell[ 9] = y*z*(1-cos(angleRadians))-x*sin(angleRadians);
	m2.cell[10] = pow(z, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[11] = 0;
	m2.cell[12] = 0;
	m2.cell[13] = 0;
	m2.cell[14] = 0;
	m2.cell[15] = 1;
	#else
	m2.cell[ 0] = pow(x, 2) *(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[ 1] = x*y*(1-cos(angleRadians))-z*sin(angleRadians);
	m2.cell[ 2] = x*z*(1-cos(angleRadians))+y*sin(angleRadians);
	m2.cell[ 3] = 0;
	m2.cell[ 4] = y*x*(1-cos(angleRadians))+z*sin(angleRadians);
	m2.cell[ 5] = pow(y, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[ 6] = y*z*(1-cos(angleRadians))-x*sin(angleRadians);
	m2.cell[ 7] = 0;
	m2.cell[ 8] = x*z*(1-cos(angleRadians))-y*sin(angleRadians);
	m2.cell[ 9] = y*z*(1-cos(angleRadians))+x*sin(angleRadians);
	m2.cell[10] = pow(z, 2)*(1-cos(angleRadians))+cos(angleRadians);
	m2.cell[11] = 0;
	m2.cell[12] = 0;
	m2.cell[13] = 0;
	m2.cell[14] = 0;
	m2.cell[15] = 1;
	#endif
	dd_matrix_mult(m, &m2);
}
/* Matrix multiplication */
void dd_matrix_mult(struct dd_matrix *m1, struct dd_matrix *m2) {
	struct dd_matrix new_mat;
	int x;
	for (x = 0; x < 16; x++) {
		#if defined( AVDL_DIRECT3D11 ) || defined( AVDL_QUEST2 )
		new_mat.cell[x] =
			(m2->cell[(x%4)+ 0] *m1->cell[(x/4)*4+0]) +
			(m2->cell[(x%4)+ 4] *m1->cell[(x/4)*4+1]) +
			(m2->cell[(x%4)+ 8] *m1->cell[(x/4)*4+2]) +
			(m2->cell[(x%4)+12] *m1->cell[(x/4)*4+3]);
		#else
		new_mat.cell[x] =
			(m1->cell[(x%4)+ 0] *m2->cell[(x/4)*4+0]) +
			(m1->cell[(x%4)+ 4] *m2->cell[(x/4)*4+1]) +
			(m1->cell[(x%4)+ 8] *m2->cell[(x/4)*4+2]) +
			(m1->cell[(x%4)+12] *m2->cell[(x/4)*4+3]);
		#endif
	}

	for (x = 0; x < 16; x++) {
		m1->cell[x] = new_mat.cell[x];
	}
}

int dd_matrix_inverse(struct dd_matrix *m) {

	struct dd_matrix m2;
	dd_matrix_create(&m2);
	dd_matrix_identity(&m2);

	m2.cell[0] = m->cell[5]  * m->cell[10] * m->cell[15] -
		m->cell[5]  * m->cell[11] * m->cell[14] -
		m->cell[9]  * m->cell[6]  * m->cell[15] +
		m->cell[9]  * m->cell[7]  * m->cell[14] +
		m->cell[13] * m->cell[6]  * m->cell[11] -
		m->cell[13] * m->cell[7]  * m->cell[10];

	m2.cell[4] = -m->cell[4]  * m->cell[10] * m->cell[15] +
		m->cell[4]  * m->cell[11] * m->cell[14] +
		m->cell[8]  * m->cell[6]  * m->cell[15] -
		m->cell[8]  * m->cell[7]  * m->cell[14] -
		m->cell[12] * m->cell[6]  * m->cell[11] +
		m->cell[12] * m->cell[7]  * m->cell[10];

	m2.cell[8] = m->cell[4]  * m->cell[9] * m->cell[15] -
		m->cell[4]  * m->cell[11] * m->cell[13] -
		m->cell[8]  * m->cell[5] * m->cell[15] +
		m->cell[8]  * m->cell[7] * m->cell[13] +
		m->cell[12] * m->cell[5] * m->cell[11] -
		m->cell[12] * m->cell[7] * m->cell[9];

	m2.cell[12] = -m->cell[4]  * m->cell[9] * m->cell[14] +
		m->cell[4]  * m->cell[10] * m->cell[13] +
		m->cell[8]  * m->cell[5] * m->cell[14] -
		m->cell[8]  * m->cell[6] * m->cell[13] -
		m->cell[12] * m->cell[5] * m->cell[10] +
		m->cell[12] * m->cell[6] * m->cell[9];

	m2.cell[1] = -m->cell[1]  * m->cell[10] * m->cell[15] +
		m->cell[1]  * m->cell[11] * m->cell[14] +
		m->cell[9]  * m->cell[2] * m->cell[15] -
		m->cell[9]  * m->cell[3] * m->cell[14] -
		m->cell[13] * m->cell[2] * m->cell[11] +
		m->cell[13] * m->cell[3] * m->cell[10];

	m2.cell[5] = m->cell[0]  * m->cell[10] * m->cell[15] -
		m->cell[0]  * m->cell[11] * m->cell[14] -
		m->cell[8]  * m->cell[2] * m->cell[15] +
		m->cell[8]  * m->cell[3] * m->cell[14] +
		m->cell[12] * m->cell[2] * m->cell[11] -
		m->cell[12] * m->cell[3] * m->cell[10];

	m2.cell[9] = -m->cell[0]  * m->cell[9] * m->cell[15] +
		m->cell[0]  * m->cell[11] * m->cell[13] +
		m->cell[8]  * m->cell[1] * m->cell[15] -
		m->cell[8]  * m->cell[3] * m->cell[13] -
		m->cell[12] * m->cell[1] * m->cell[11] +
		m->cell[12] * m->cell[3] * m->cell[9];

	m2.cell[13] = m->cell[0]  * m->cell[9] * m->cell[14] -
		m->cell[0]  * m->cell[10] * m->cell[13] -
		m->cell[8]  * m->cell[1] * m->cell[14] +
		m->cell[8]  * m->cell[2] * m->cell[13] +
		m->cell[12] * m->cell[1] * m->cell[10] -
		m->cell[12] * m->cell[2] * m->cell[9];

	m2.cell[2] = m->cell[1]  * m->cell[6] * m->cell[15] -
		m->cell[1]  * m->cell[7] * m->cell[14] -
		m->cell[5]  * m->cell[2] * m->cell[15] +
		m->cell[5]  * m->cell[3] * m->cell[14] +
		m->cell[13] * m->cell[2] * m->cell[7] -
		m->cell[13] * m->cell[3] * m->cell[6];

	m2.cell[6] = -m->cell[0]  * m->cell[6] * m->cell[15] +
		m->cell[0]  * m->cell[7] * m->cell[14] +
		m->cell[4]  * m->cell[2] * m->cell[15] -
		m->cell[4]  * m->cell[3] * m->cell[14] -
		m->cell[12] * m->cell[2] * m->cell[7] +
		m->cell[12] * m->cell[3] * m->cell[6];

	m2.cell[10] = m->cell[0]  * m->cell[5] * m->cell[15] -
		m->cell[0]  * m->cell[7] * m->cell[13] -
		m->cell[4]  * m->cell[1] * m->cell[15] +
		m->cell[4]  * m->cell[3] * m->cell[13] +
		m->cell[12] * m->cell[1] * m->cell[7] -
		m->cell[12] * m->cell[3] * m->cell[5];

	m2.cell[14] = -m->cell[0]  * m->cell[5] * m->cell[14] +
		m->cell[0]  * m->cell[6] * m->cell[13] +
		m->cell[4]  * m->cell[1] * m->cell[14] -
		m->cell[4]  * m->cell[2] * m->cell[13] -
		m->cell[12] * m->cell[1] * m->cell[6] +
		m->cell[12] * m->cell[2] * m->cell[5];

	m2.cell[3] = -m->cell[1] * m->cell[6] * m->cell[11] +
		m->cell[1] * m->cell[7] * m->cell[10] +
		m->cell[5] * m->cell[2] * m->cell[11] -
		m->cell[5] * m->cell[3] * m->cell[10] -
		m->cell[9] * m->cell[2] * m->cell[7] +
		m->cell[9] * m->cell[3] * m->cell[6];

	m2.cell[7] = m->cell[0] * m->cell[6] * m->cell[11] -
		m->cell[0] * m->cell[7] * m->cell[10] -
		m->cell[4] * m->cell[2] * m->cell[11] +
		m->cell[4] * m->cell[3] * m->cell[10] +
		m->cell[8] * m->cell[2] * m->cell[7] -
		m->cell[8] * m->cell[3] * m->cell[6];

	m2.cell[11] = -m->cell[0] * m->cell[5] * m->cell[11] +
		m->cell[0] * m->cell[7] * m->cell[9] +
		m->cell[4] * m->cell[1] * m->cell[11] -
		m->cell[4] * m->cell[3] * m->cell[9] -
		m->cell[8] * m->cell[1] * m->cell[7] +
		m->cell[8] * m->cell[3] * m->cell[5];

	m2.cell[15] = m->cell[0] * m->cell[5] * m->cell[10] -
		m->cell[0] * m->cell[6] * m->cell[9] -
		m->cell[4] * m->cell[1] * m->cell[10] +
		m->cell[4] * m->cell[2] * m->cell[9] +
		m->cell[8] * m->cell[1] * m->cell[6] -
		m->cell[8] * m->cell[2] * m->cell[5];

	float det = m->cell[0] * m2.cell[0] + m->cell[1] * m2.cell[4] + m->cell[2] * m2.cell[8] + m->cell[3] * m2.cell[12];

	// can't inverse
	if (det == 0) {
		return 0;
	}

	// inversed
	det = 1.0 / det;
	for (int i = 0; i < 16; i++) {
		m->cell[i] = m2.cell[i] * det;
	}
	return 1;
}

void dd_matrix_copy(struct dd_matrix *m1, struct dd_matrix *m2) {
	memcpy(m1->cell, m2->cell, sizeof(float) *16);
}

void dd_matrix_copy_rs(struct dd_matrix *m1, struct dd_matrix *m2) {
	memcpy(m1->cell, m2->cell, sizeof(float) *12);
	m1->cell[15] = m2->cell[15];
}

void dd_matrix_approach(struct dd_matrix *m, struct dd_matrix *target, float counter) {
	for (int i = 0; i < 16; i++) {
		m->cell[i] = m->cell[i] +(target->cell[i] -m->cell[i]) *counter;
	}
}

void dd_matrix_approach_rs(struct dd_matrix *m, struct dd_matrix *target, float counter) {
	for (int i = 0; i < 12; i++) {
		m->cell[i] = m->cell[i] +(target->cell[i] -m->cell[i]) *counter;
	}
	m->cell[15] = m->cell[15] +(target->cell[15] -m->cell[15]) *counter;
}

/* Getters */
float dd_matrix_x(struct dd_matrix *m) { return m->cell[12]; }
float dd_matrix_y(struct dd_matrix *m) { return m->cell[13]; }
float dd_matrix_z(struct dd_matrix *m) { return m->cell[14]; }

/* Print the matrix, this is clearly meant for debugging only,
	don't use it in production code, seriously
 */
void dd_matrix_print(struct dd_matrix *m) {
	int i;
	for (i = 0; i < 16; i += 4) {
		avdl_log("%f %f %f %f",
			m->cell[(i) +0],
			m->cell[(i) +1],
			m->cell[(i) +2],
			m->cell[(i) +3]
		);

		if ((i -3) % 4 == 0) {
			avdl_log("");
		}
	}
}

/*
 * custom matrix stack implementation
 */
struct dd_matrix dd_cam[DD_MATRIX_STACK_LIMIT];
int dd_cam_index = 0;
extern struct dd_matrix matPerspective;
struct dd_matrix matView;
struct dd_matrix matModel[DD_MATRIX_STACK_LIMIT];
int matModel_index = 0;

void dd_matrix_globalInit() {
	dd_cam_index = 0;
	matModel_index = 0;

	// for the time being, on quest 2, the engine is giving the default matrix
	// (where the headset is) so don't create perspective
	#if defined( AVDL_QUEST2 )
	dd_matrix_identity(&dd_cam[0]);
	#else
	dd_matrix_copy(&dd_cam[0], &matPerspective);
	dd_matrix_identity(&matView);
	dd_matrix_identity(&matModel[0]);
	#endif
}

void dd_matrix_push() {
	if (dd_cam_index < (DD_MATRIX_STACK_LIMIT -1)) {
		dd_cam_index++;
		dd_matrix_copy(&dd_cam[dd_cam_index], &dd_cam[dd_cam_index-1]);
	}
	else {
		avdl_log("dd_matrix: maximum matrix stack reached");
		exit(-1);
	}
	if (matModel_index < (DD_MATRIX_STACK_LIMIT -1)) {
		matModel_index++;
		dd_matrix_copy(&matModel[matModel_index], &matModel[matModel_index-1]);
	}
	else {
		avdl_log("dd_matrix matModel: maximum matrix stack reached");
		exit(-1);
	}
}

void dd_matrix_pop() {
	if (dd_cam_index > 0) {
		dd_cam_index--;
	}
	else {
		avdl_log("dd_matrix: no matrix to pop");
		exit(-1);
	}
	if (matModel_index > 0) {
		matModel_index--;
	}
	else {
		avdl_log("dd_matrix: no matrix model to pop");
		exit(-1);
	}
}

void dd_translatef(float x, float y, float z) {
	dd_matrix_translatem(dd_matrix_globalGet(), x, y, z);
	dd_matrix_translatem(&matModel[matModel_index], x, y, z);
}

void dd_scalef(float x, float y, float z) {
	dd_matrix_scalem(dd_matrix_globalGet(), x, y, z);
	dd_matrix_scalem(&matModel[matModel_index], x, y, z);
}

void dd_rotatef(float angle, float x, float y, float z) {
	dd_matrix_rotatem(dd_matrix_globalGet(), angle, x, y, z);
	dd_matrix_rotatem(&matModel[matModel_index], angle, x, y, z);
}

void dd_multMatrixf(struct dd_matrix *matrix) {
	dd_matrix_mult(dd_matrix_globalGet(), matrix);
	dd_matrix_mult(&matModel[matModel_index], matrix);
}

// camera translations
void dd_translatef_camera(float x, float y, float z) {
	dd_matrix_translatem(dd_matrix_globalGet(), x, y, z);
	dd_matrix_translatem(&matView, x, y, z);
}

void dd_scalef_camera(float x, float y, float z) {
	dd_matrix_scalem(dd_matrix_globalGet(), x, y, z);
	dd_matrix_scalem(&matView, x, y, z);
}

void dd_rotatef_camera(float angle, float x, float y, float z) {
	dd_matrix_rotatem(dd_matrix_globalGet(), angle, x, y, z);
	dd_matrix_rotatem(&matView, angle, x, y, z);
}

void dd_multMatrixf_camera(struct dd_matrix *matrix) {
	dd_matrix_mult(dd_matrix_globalGet(), matrix);
	dd_matrix_mult(&matView, matrix);
}

struct dd_matrix *dd_matrix_globalGet() {
	return &dd_cam[dd_cam_index];
}

/*
 * lookat function
 *
 * make given matrix look at target (local space)
 */
void dd_matrix_lookat(struct dd_matrix *m, float targetX, float targetY, float targetZ) {

	struct avdl_vec3 forward;
	avdl_vec3_create(&forward);
	avdl_vec3_Setf(&forward, targetX, targetY, targetZ);
	avdl_vec3_Normalise(&forward);

	struct avdl_vec3 fakeup;
	avdl_vec3_create(&fakeup);
	avdl_vec3_Setf(&fakeup, 0, 1, 0);

	struct avdl_vec3 right;
	avdl_vec3_create(&right);
	avdl_vec3_Set(&right, &fakeup);
	avdl_vec3_Cross(&right, &forward);
	avdl_vec3_Normalise(&right);

	struct avdl_vec3 up;
	avdl_vec3_create(&up);
	avdl_vec3_Set(&up, &forward);
	avdl_vec3_Cross(&up, &right);
	avdl_vec3_Normalise(&up);

	#if defined( AVDL_DIRECT3D11 ) || defined( AVDL_QUEST2 )
	float rot_mat[] = {
		right.x, up.x, forward.x, 0,
		right.y, up.y, forward.y, 0,
		right.z, up.z, forward.z, 0,
		0, 0, 0, 1
	};
	#else
	float rot_mat[] = {
		right.x, right.y, right.z, 0,
		up.x, up.y, up.z, 0,
		forward.x, forward.y, forward.z, 0,
		0, 0, 0, 1,
	};
	#endif

	for (int i = 0; i < 16; i++) {
		m->cell[i] = rot_mat[i];
	}

}

