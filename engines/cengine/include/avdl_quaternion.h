#ifndef AVDL_QUATERNION_H
#define AVDL_QUATERNION_H

#ifdef __cplusplus
extern "C" {
#endif

struct dd_matrix;
struct avdl_vec3;


struct avdl_quaternion {
	float x;
	float y;
	float z;
	float w;
};

void avdl_quaternion_create(struct avdl_quaternion *o);

void avdl_quaternion_Identity(struct avdl_quaternion *o);
void avdl_quaternion_RotationFromEuler(struct avdl_quaternion *o, float x, float y, float z);
void avdl_quaternion_Multiply(struct avdl_quaternion *o, struct avdl_quaternion *q);
void avdl_quaternion_ToRotationMatrix(struct avdl_quaternion *o, struct dd_matrix *m);
void avdl_quaternion_ToEuler(struct avdl_quaternion *o, struct avdl_vec3 *v);
void avdl_quaternion_Copy(struct avdl_quaternion *o, struct avdl_quaternion *q);
void avdl_quaternion_Conjugate(struct avdl_quaternion *o);

void avdl_quaternion_Print(struct avdl_quaternion *o);

#ifdef __cplusplus
}
#endif

#endif
