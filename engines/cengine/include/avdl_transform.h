#ifndef AVDL_TRANSFORM_H
#define AVDL_TRANSFORM_H

#include "avdl_vec3.h"
#include "dd_matrix.h"

struct avdl_transform {

	struct avdl_vec3 position;
	struct avdl_vec3 rotation;
	struct avdl_vec3 scale;

	int matrix_dirty;
	struct dd_matrix matrix;
	int matrix_inverse_dirty;
	struct dd_matrix matrix_inverse;
	int matrix_normal_dirty;
	struct dd_matrix matrix_normal;
	int matrix_normal_inverse_dirty;
	struct dd_matrix matrix_normal_inverse;

};

void avdl_transform_SetPosition(struct avdl_transform *o, struct avdl_vec3 *src);
void avdl_transform_SetPosition3f(struct avdl_transform *o, float x, float y, float z);
void avdl_transform_SetRotation(struct avdl_transform *o, struct avdl_vec3 *src);
void avdl_transform_SetRotation3f(struct avdl_transform *o, float x, float y, float z);
void avdl_transform_SetScale(struct avdl_transform *o, struct avdl_vec3 *src);
void avdl_transform_SetScale3f(struct avdl_transform *o, float x, float y, float z);

struct dd_matrix *avdl_transform_GetMatrix(struct avdl_transform *o);
struct dd_matrix *avdl_transform_GetInverseMatrix(struct avdl_transform *o);
struct dd_matrix *avdl_transform_GetNormalMatrix(struct avdl_transform *o);
struct dd_matrix *avdl_transform_GetNormalInverseMatrix(struct avdl_transform *o);
struct avdl_vec3 *avdl_transform_GetPosition(struct avdl_transform *o);
float avdl_transform_GetPositionX(struct avdl_transform *o);
float avdl_transform_GetPositionY(struct avdl_transform *o);
float avdl_transform_GetPositionZ(struct avdl_transform *o);
struct avdl_vec3 *avdl_transform_GetRotation(struct avdl_transform *o);
float avdl_transform_GetRotationX(struct avdl_transform *o);
float avdl_transform_GetRotationY(struct avdl_transform *o);
float avdl_transform_GetRotationZ(struct avdl_transform *o);
struct avdl_vec3 *avdl_transform_GetScale(struct avdl_transform *o);
float avdl_transform_GetScaleX(struct avdl_transform *o);
float avdl_transform_GetScaleY(struct avdl_transform *o);
float avdl_transform_GetScaleZ(struct avdl_transform *o);

int avdl_transform_Copy(struct avdl_transform *o, struct avdl_transform *target);

void avdl_transform_create(struct avdl_transform *o);

int avdl_transform_MultiplyMatrix(struct avdl_transform *o, struct dd_matrix *matrix);

#endif
