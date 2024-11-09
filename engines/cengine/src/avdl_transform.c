#include "avdl_transform.h"

void avdl_transform_create(struct avdl_transform *o) {
	o->clean = avdl_transform_clean;
	o->SetPosition = avdl_transform_SetPosition;
	o->SetPosition3f = avdl_transform_SetPosition3f;
	o->SetRotation = avdl_transform_SetRotation;
	o->SetRotation3f = avdl_transform_SetRotation3f;
	o->SetScale = avdl_transform_SetScale;
	o->SetScale3f = avdl_transform_SetScale3f;

	o->GetMatrix = avdl_transform_GetMatrix;
	o->GetInverseMatrix = avdl_transform_GetInverseMatrix;
	o->GetPosition = avdl_transform_GetPosition;
	o->GetRotation = avdl_transform_GetRotation;
	o->GetScale = avdl_transform_GetScale;

	dd_vec3_setf(&o->position, 0, 0, 0);
	dd_vec3_setf(&o->rotation, 0, 0, 0);
	dd_vec3_setf(&o->scale, 1, 1, 1);
	dd_matrix_identity(&o->matrix);
	dd_matrix_identity(&o->matrix_inverse);

	o->matrix_dirty = 0;
	o->matrix_inverse_dirty = 0;
}

void avdl_transform_clean(struct avdl_transform *o) {
}

void avdl_transform_SetPosition(struct avdl_transform *o, struct dd_vec3 *src) {
	dd_vec3_set(&o->position, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetPosition3f(struct avdl_transform *o, float x, float y, float z) {
	dd_vec3_setf(&o->position, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetRotation(struct avdl_transform *o, struct dd_vec3 *src) {
	dd_vec3_set(&o->rotation, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetRotation3f(struct avdl_transform *o, float x, float y, float z) {
	dd_vec3_setf(&o->rotation, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetScale(struct avdl_transform *o, struct dd_vec3 *src) {
	dd_vec3_set(&o->scale, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetScale3f(struct avdl_transform *o, float x, float y, float z) {
	dd_vec3_setf(&o->scale, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

struct dd_matrix *avdl_transform_GetMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_dirty) {
		dd_matrix_identity(&o->matrix);
		dd_matrix_translate(&o->matrix,
			dd_vec3_getX(&o->position),
			dd_vec3_getY(&o->position),
			dd_vec3_getZ(&o->position)
		);
		dd_matrix_rotate(&o->matrix, dd_vec3_getZ(&o->rotation), 0, 0, 1);
		dd_matrix_rotate(&o->matrix, dd_vec3_getY(&o->rotation), 0, 1, 0);
		dd_matrix_rotate(&o->matrix, dd_vec3_getX(&o->rotation), 1, 0, 0);
		dd_matrix_scale(&o->matrix,
			dd_vec3_getX(&o->scale),
			dd_vec3_getY(&o->scale),
			dd_vec3_getZ(&o->scale)
		);
		o->matrix_dirty	= 0;
	}
	return &o->matrix;
}

struct dd_matrix *avdl_transform_GetInverseMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_inverse_dirty) {
		dd_matrix_copy(&o->matrix_inverse, avdl_transform_GetMatrix(o));
		dd_matrix_inverse(&o->matrix_inverse);
		o->matrix_inverse_dirty	= 0;
	}
	return &o->matrix_inverse;
}

struct dd_vec3 *avdl_transform_GetPosition(struct avdl_transform *o) {
	return &o->position;
}

struct dd_vec3 *avdl_transform_GetRotation(struct avdl_transform *o) {
	return &o->rotation;
}

struct dd_vec3 *avdl_transform_GetScale(struct avdl_transform *o) {
	return &o->scale;
}
