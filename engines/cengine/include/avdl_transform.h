#ifndef AVDL_TRANSFORM_H
#define AVDL_TRANSFORM_H

#include "dd_vec3.h"
#include "dd_matrix.h"

struct avdl_transform {

	struct dd_vec3 position;
	struct dd_vec3 rotation;
	struct dd_vec3 scale;

	int matrix_dirty;
	struct dd_matrix matrix;
	int matrix_inverse_dirty;
	struct dd_matrix matrix_inverse;
	int matrix_normal_dirty;
	struct dd_matrix matrix_normal;

	// setters
	void (*SetPosition)(struct avdl_transform *, struct dd_vec3 *src);
	void (*SetPosition3f)(struct avdl_transform *, float x, float y, float z);
	void (*SetRotation)(struct avdl_transform *, struct dd_vec3 *src);
	void (*SetRotation3f)(struct avdl_transform *, float x, float y, float z);
	void (*SetScale)(struct avdl_transform *, struct dd_vec3 *src);
	void (*SetScale3f)(struct avdl_transform *, float x, float y, float z);

	// getters
	struct dd_matrix *(*GetMatrix)(struct avdl_transform *);
	struct dd_matrix *(*GetInverseMatrix)(struct avdl_transform *);
	struct dd_matrix *(*GetNormalMatrix)(struct avdl_transform *);
	struct dd_vec3 *(*GetPosition)(struct avdl_transform *);
	struct dd_vec3 *(*GetRotation)(struct avdl_transform *);
	struct dd_vec3 *(*GetScale)(struct avdl_transform *);

	void (*create)(struct avdl_transform *);
	void (*clean)(struct avdl_transform *);

};

void avdl_transform_SetPosition(struct avdl_transform *o, struct dd_vec3 *src);
void avdl_transform_SetPosition3f(struct avdl_transform *o, float x, float y, float z);
void avdl_transform_SetRotation(struct avdl_transform *o, struct dd_vec3 *src);
void avdl_transform_SetRotation3f(struct avdl_transform *o, float x, float y, float z);
void avdl_transform_SetScale(struct avdl_transform *o, struct dd_vec3 *src);
void avdl_transform_SetScale3f(struct avdl_transform *o, float x, float y, float z);

struct dd_matrix *avdl_transform_GetMatrix(struct avdl_transform *o);
struct dd_matrix *avdl_transform_GetInverseMatrix(struct avdl_transform *o);
struct dd_matrix *avdl_transform_GetNormalMatrix(struct avdl_transform *o);
struct dd_vec3 *avdl_transform_GetPosition(struct avdl_transform *o);
struct dd_vec3 *avdl_transform_GetRotation(struct avdl_transform *o);
struct dd_vec3 *avdl_transform_GetScale(struct avdl_transform *o);

void avdl_transform_create(struct avdl_transform *o);
void avdl_transform_clean(struct avdl_transform *o);

#endif
