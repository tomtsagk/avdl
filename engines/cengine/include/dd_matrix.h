#ifndef DD_MATRIX_H
#define DD_MATRIX_H

/* a series of functions to manipulate a 4x4 matrix.
 * these functions allow transformation, scale and rotation (experimental)
 */
struct dd_matrix {
	float cell[16];
};

void dd_matrix_create(float mat[16]);

/* identity matrix
 * identity  : makes an identity matrix
 * identityt : makes an identity matrix, except from translation
 	(so it cancels rotation and scale)
 */
void dd_matrix_identity (float mat[16]);
void dd_matrix_identityt(float mat[16]);

/* Translate - Seperated in add, set and multiply 
 * add : adds fixed translation to the matrix
 * set : sets (overwrites) fixed translation to the matrix
 * multiply : creates a new matrix with given translation and
 	multiplies the given matrix with it
 */
void dd_matrix_translatea(float mat[16], float x, float y, float z);
void dd_matrix_translates(float mat[16], float x, float y, float z);
void dd_matrix_translatem(float mat[16], float x, float y, float z);

/* Scale - Seperated in add and set */
void dd_matrix_scalea(float mat[16], float x, float y, float z);
void dd_matrix_scales(float mat[16], float x, float y, float z);
void dd_matrix_scalem(float mat[16], float x, float y, float z);

/* Rotate - experimental functions */
void dd_matrix_rotate_x(float mat[16], float rad);
void dd_matrix_rotate_y(float mat[16], float rad);
void dd_matrix_rotate_z(float mat[16], float rad);
void dd_matrix_rotatelocal_x(float mat[16], float rad);
void dd_matrix_rotatelocal_y(float mat[16], float rad);
void dd_matrix_rotatelocal_z(float mat[16], float rad);
void dd_matrix_rotatem(float mat[16], float rad, float x, float y, float z);

/* Matrix multiplication */
void dd_matrix_mult(float mat1[16], float mat2[16]);

void dd_matrix_copy(float mat1[16], float mat2[16]);
void dd_matrix_copy_rs(float mat1[16], float mat2[16]);

void dd_matrix_approach(float mat[16], float target[16], float counter);
void dd_matrix_approach_rs(float mat[16], float target[16], float counter);

/* Getters */
float dd_matrix_x(float mat[16]);
float dd_matrix_y(float mat[16]);
float dd_matrix_z(float mat[16]);

/* Print matrix - don't use this */
void dd_matrix_print(float mat[16]);

/*
 * manual implementation of the matrix stack
 */
#define DD_MATRIX_STACK_LIMIT 10
extern struct dd_matrix dd_cam[];
extern int dd_cam_index;

void dd_matrix_globalInit();
void dd_matrix_push();
void dd_matrix_pop ();
struct dd_matrix *dd_matrix_globalGet();

#define dd_pushMatrix() dd_matrix_push()
#define dd_popMatrix() dd_matrix_pop()
#define dd_translatef(x, y, z) dd_matrix_translatem((float*)dd_matrix_globalGet(), x, y, z)
#define dd_scalef(x, y, z) dd_matrix_scalem((float*)dd_matrix_globalGet(), x, y, z)
#define dd_rotatef(angle, x, y, z) dd_matrix_rotatem((float*)dd_matrix_globalGet(), angle, x, y, z)
#define dd_multMatrixf(matrix) dd_matrix_mult((float*)dd_matrix_globalGet(), matrix);

#endif
