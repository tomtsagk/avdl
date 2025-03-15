#include "avdl_transform.h"
#include "avdl_log.h"

void avdl_transform_create(struct avdl_transform *o) {

	avdl_vec3_Setf(&o->position, 0, 0, 0);
	avdl_vec3_Setf(&o->rotation, 0, 0, 0);
	avdl_vec3_Setf(&o->scale, 1, 1, 1);
	dd_matrix_identity(&o->matrix);
	dd_matrix_identity(&o->matrix_inverse);
	dd_matrix_identity(&o->matrix_normal);
	dd_matrix_identity(&o->matrix_normal_inverse);

	o->matrix_dirty = 0;
	o->matrix_inverse_dirty = 0;
	o->matrix_normal_dirty = 0;
	o->matrix_normal_inverse_dirty = 0;
}

void avdl_transform_SetPosition(struct avdl_transform *o, struct avdl_vec3 *src) {
	avdl_vec3_Set(&o->position, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetPosition3f(struct avdl_transform *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->position, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
}

void avdl_transform_SetRotation(struct avdl_transform *o, struct avdl_vec3 *src) {
	avdl_vec3_Set(&o->rotation, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
	o->matrix_normal_dirty = 1;
	o->matrix_normal_inverse_dirty = 1;
}

void avdl_transform_SetRotation3f(struct avdl_transform *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->rotation, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
	o->matrix_normal_dirty = 1;
	o->matrix_normal_inverse_dirty = 1;
}

void avdl_transform_SetScale(struct avdl_transform *o, struct avdl_vec3 *src) {
	avdl_vec3_Set(&o->scale, src);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
	o->matrix_normal_inverse_dirty = 1;
}

void avdl_transform_SetScale3f(struct avdl_transform *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->scale, x, y, z);
	o->matrix_dirty = 1;
	o->matrix_inverse_dirty = 1;
	o->matrix_normal_inverse_dirty = 1;
}

struct dd_matrix *avdl_transform_GetMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_dirty) {
		dd_matrix_identity(&o->matrix);
		dd_matrix_translate(&o->matrix,
			avdl_vec3_X(&o->position),
			avdl_vec3_Y(&o->position),
			avdl_vec3_Z(&o->position)
		);
		dd_matrix_rotate(&o->matrix, avdl_vec3_Z(&o->rotation), 0, 0, 1);
		dd_matrix_rotate(&o->matrix, avdl_vec3_Y(&o->rotation), 0, 1, 0);
		dd_matrix_rotate(&o->matrix, avdl_vec3_X(&o->rotation), 1, 0, 0);
		dd_matrix_scale(&o->matrix,
			avdl_vec3_X(&o->scale),
			avdl_vec3_Y(&o->scale),
			avdl_vec3_Z(&o->scale)
		);
		o->matrix_dirty	= 0;
	}
	return &o->matrix;
}

struct dd_matrix *avdl_transform_GetInverseMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_inverse_dirty) {
		//dd_matrix_copy(&o->matrix_inverse, avdl_transform_GetMatrix(o));
		//dd_matrix_inverse(&o->matrix_inverse);

		dd_matrix_identity(&o->matrix_inverse);
		dd_matrix_scale(&o->matrix_inverse,
			1/avdl_vec3_X(&o->scale),
			1/avdl_vec3_Y(&o->scale),
			1/avdl_vec3_Z(&o->scale)
		);
		dd_matrix_rotate(&o->matrix_inverse, -avdl_vec3_X(&o->rotation), 1, 0, 0);
		dd_matrix_rotate(&o->matrix_inverse, -avdl_vec3_Y(&o->rotation), 0, 1, 0);
		dd_matrix_rotate(&o->matrix_inverse, -avdl_vec3_Z(&o->rotation), 0, 0, 1);
		dd_matrix_translate(&o->matrix_inverse,
			-avdl_vec3_X(&o->position),
			-avdl_vec3_Y(&o->position),
			-avdl_vec3_Z(&o->position)
		);
		o->matrix_inverse_dirty	= 0;
	}
	return &o->matrix_inverse;
}

struct dd_matrix *avdl_transform_GetNormalMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_normal_dirty) {
		dd_matrix_identity(&o->matrix_normal);
		dd_matrix_rotate(&o->matrix_normal, avdl_vec3_Z(&o->rotation), 0, 0, 1);
		dd_matrix_rotate(&o->matrix_normal, avdl_vec3_Y(&o->rotation), 0, 1, 0);
		dd_matrix_rotate(&o->matrix_normal, avdl_vec3_X(&o->rotation), 1, 0, 0);
		o->matrix_normal_dirty	= 0;
	}
	return &o->matrix_normal;
}

struct dd_matrix *avdl_transform_GetNormalInverseMatrix(struct avdl_transform *o) {
	// re-calculate matrix
	if (o->matrix_normal_inverse_dirty) {
		dd_matrix_identity(&o->matrix_normal_inverse);
		dd_matrix_rotate(&o->matrix_normal_inverse, -avdl_vec3_X(&o->rotation), 1, 0, 0);
		dd_matrix_rotate(&o->matrix_normal_inverse, -avdl_vec3_Y(&o->rotation), 0, 1, 0);
		dd_matrix_rotate(&o->matrix_normal_inverse, -avdl_vec3_Z(&o->rotation), 0, 0, 1);
		o->matrix_normal_inverse_dirty	= 0;
	}
	return &o->matrix_normal_inverse;
}

struct avdl_vec3 *avdl_transform_GetPosition(struct avdl_transform *o) {
	return &o->position;
}

struct avdl_vec3 *avdl_transform_GetRotation(struct avdl_transform *o) {
	return &o->rotation;
}

struct avdl_vec3 *avdl_transform_GetScale(struct avdl_transform *o) {
	return &o->scale;
}

int avdl_transform_Copy(struct avdl_transform *o, struct avdl_transform *target) {
	avdl_transform_SetPosition(o, &target->position);
	avdl_transform_SetRotation(o, &target->rotation);
	avdl_transform_SetScale(o, &target->scale);
	return 0;
}
