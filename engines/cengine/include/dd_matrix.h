#ifndef DD_MATRIX_H
#define DD_MATRIX_H

/*
 * matrix, that controls transformations of objects
 */
struct dd_matrix {
	float cell[16];
};
void dd_matrix_create(struct dd_matrix *m);
void dd_matrix_clean(struct dd_matrix *m);

/*
 * matrix setters
 */
void dd_matrix_identity(struct dd_matrix *m);
void dd_matrix_copy(struct dd_matrix *m1, struct dd_matrix *m2);
void dd_matrix_mult(struct dd_matrix *m1, struct dd_matrix *m2);

/*
 * matrix transformations: translate, scale, rotate, approach
 */
void dd_matrix_translate(struct dd_matrix *m, float x, float y, float z);
void dd_matrix_scale    (struct dd_matrix *m, float x, float y, float z);
void dd_matrix_rotate   (struct dd_matrix *m, float rad, float x, float y, float z);
void dd_matrix_approach (struct dd_matrix *m, struct dd_matrix *target, float counter);

/*
 * print matrix - only for debugging reasons
 */
void dd_matrix_print(struct dd_matrix *m);

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
#define dd_translatef(x, y, z) dd_matrix_translatem(dd_matrix_globalGet(), x, y, z)
#define dd_scalef(x, y, z) dd_matrix_scalem(dd_matrix_globalGet(), x, y, z)
#define dd_rotatef(angle, x, y, z) dd_matrix_rotatem(dd_matrix_globalGet(), angle, x, y, z)
#define dd_multMatrixf(matrix) dd_matrix_mult(dd_matrix_globalGet(), matrix);

void dd_matrix_lookat(struct dd_matrix *m, float targetX, float targetY, float targetZ);

/*
 * deprecated functions
 *
 * they shouldn't be used anymore,
 * will be removed in a future version.
 */
void dd_matrix_identityt(struct dd_matrix *m);
void dd_matrix_translatea(struct dd_matrix *m, float x, float y, float z);
void dd_matrix_translates(struct dd_matrix *m, float x, float y, float z);
#define dd_matrix_translatem(m, x, y, z) dd_matrix_translate(m, x, y, z)
void dd_matrix_scalea(struct dd_matrix *m, float x, float y, float z);
void dd_matrix_scales(struct dd_matrix *m, float x, float y, float z);
#define dd_matrix_scalem(m, x, y, z) dd_matrix_scale(m, x, y, z)
void dd_matrix_rotate_x(struct dd_matrix *m, float rad);
void dd_matrix_rotate_y(struct dd_matrix *m, float rad);
void dd_matrix_rotate_z(struct dd_matrix *m, float rad);
void dd_matrix_rotatelocal_x(struct dd_matrix *m, float rad);
void dd_matrix_rotatelocal_y(struct dd_matrix *m, float rad);
void dd_matrix_rotatelocal_z(struct dd_matrix *m, float rad);
#define dd_matrix_rotatem(m, rad, x, y, z) dd_matrix_rotate(m, rad, x, y, z)
void dd_matrix_copy_rs(struct dd_matrix *m1, struct dd_matrix *m2);
void dd_matrix_approach_rs(struct dd_matrix *m1, struct dd_matrix *m2, float counter);
float dd_matrix_x(struct dd_matrix *m);
float dd_matrix_y(struct dd_matrix *m);
float dd_matrix_z(struct dd_matrix *m);

#endif
