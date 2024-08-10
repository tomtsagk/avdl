#ifndef DD_MATRIX_H
#define DD_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * matrix, that controls transformations of objects
 */
struct dd_matrix {
	float cell[16];
};
void dd_matrix_create(struct dd_matrix *m);
void dd_matrix_clean(struct dd_matrix *m);

#include "dd_vec4.h"

/*
 * matrix setters
 */
void dd_matrix_identity(struct dd_matrix *m);
void dd_matrix_copy(struct dd_matrix *m1, struct dd_matrix *m2);
void dd_matrix_mult(struct dd_matrix *m1, struct dd_matrix *m2);
int dd_matrix_inverse(struct dd_matrix *m);

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

#if defined(AVDL_QUEST2)
extern struct dd_matrix dd_cam_controllers[];
extern int dd_cam_controller_active[];
extern struct dd_vec4 dd_cam_controllers_position[];
extern struct dd_vec4 dd_cam_controllers_direction[];
#endif
void dd_matrix_setControllerMatrix(int controllerIndex, struct dd_matrix *m);
struct dd_matrix *dd_matrix_getControllerMatrix(int controllerIndex);
void dd_matrix_applyControllerMatrix(int controllerIndex);
int dd_matrix_hasVisibleControllers();
int dd_matrix_isControllerVisible(int index);
void dd_matrix_setControllerVisible(int index, int state);
struct dd_vec4 *dd_matrix_getControllerPosition(int index);
struct dd_vec4 *dd_matrix_getControllerDirection(int index);

void dd_matrix_globalInit();
void dd_matrix_push();
void dd_matrix_pop ();
struct dd_matrix *dd_matrix_globalGet();
struct dd_matrix *dd_matrix_globalVRGet();

#define dd_pushMatrix() dd_matrix_push()
#define dd_popMatrix() dd_matrix_pop()
void dd_translatef(float x, float y, float z);
void dd_scalef(float x, float y, float z);
void dd_rotatef(float angle, float x, float y, float z);
void dd_multMatrixf(struct dd_matrix *matrix);
void dd_translatef_camera(float x, float y, float z);
void dd_scalef_camera(float x, float y, float z);
void dd_rotatef_camera(float angle, float x, float y, float z);
void dd_multMatrixf_camera(struct dd_matrix *matrix);

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

void dd_matrix_quaternion_to_rotation_matrix(struct dd_vec4 *q, struct dd_matrix *output);

#ifdef __cplusplus
}
#endif

#endif
