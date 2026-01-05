#ifndef AVDL_VEC4_H
#define AVDL_VEC4_H

#ifdef __cplusplus
extern "C" {
#endif

struct dd_matrix;
struct avdl_vec3;

struct avdl_vec4 {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
	union {
		float w;
		float a;
	};
};

void avdl_vec4_create(struct avdl_vec4 *o);
void avdl_vec4_Setf(struct avdl_vec4 *o, float x, float y, float z, float w);
void avdl_vec4_Set1f(struct avdl_vec4 *o, float value);
void avdl_vec4_Set(struct avdl_vec4 *o, struct avdl_vec4 *o2);
void avdl_vec4_SetVec3(struct avdl_vec4 *o, struct avdl_vec3 *o2);
void avdl_vec4_SetX(struct avdl_vec4 *o, float value);
void avdl_vec4_SetY(struct avdl_vec4 *o, float value);
void avdl_vec4_SetZ(struct avdl_vec4 *o, float value);
void avdl_vec4_SetW(struct avdl_vec4 *o, float value);

// math
void avdl_vec4_Add (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Addf(struct avdl_vec4 *o1, float x, float y, float z, float w);
void avdl_vec4_Add1f(struct avdl_vec4 *o1, float value);
void avdl_vec4_AddVec3(struct avdl_vec4 *o1, struct avdl_vec3 *v);

void avdl_vec4_Subtract (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Subtractf(struct avdl_vec4 *o1, float x, float y, float z, float w);
void avdl_vec4_Subtract1f(struct avdl_vec4 *o1, float value);
void avdl_vec4_SubtractVec3(struct avdl_vec4 *o1, struct avdl_vec3 *v);

void avdl_vec4_Multiply (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Multiplyf(struct avdl_vec4 *o, float x, float y, float z, float w);
void avdl_vec4_Multiply1f(struct avdl_vec4 *o, float x);
void avdl_vec4_MultiplyVec3(struct avdl_vec4 *o, struct avdl_vec3 *v);
void avdl_vec4_MultiplyMatrix(struct avdl_vec4 *o, struct dd_matrix *mat);

void avdl_vec4_Divide (struct avdl_vec4 *o1, struct avdl_vec4 *o2);
void avdl_vec4_Dividef(struct avdl_vec4 *o, float x, float y, float z, float w);
void avdl_vec4_Divide1f(struct avdl_vec4 *o, float value);
void avdl_vec4_DivideVec3(struct avdl_vec4 *o, struct avdl_vec3 *v);

float avdl_vec4_X(struct avdl_vec4 *o);
float avdl_vec4_Y(struct avdl_vec4 *o);
float avdl_vec4_Z(struct avdl_vec4 *o);
float avdl_vec4_W(struct avdl_vec4 *o);

float avdl_vec4_Dot(struct avdl_vec4 *a, struct avdl_vec4 *b);
float avdl_vec4_DotVec3(struct avdl_vec4 *a, struct avdl_vec3 *b);
void avdl_vec4_Cross(struct avdl_vec4 *a, struct avdl_vec4 *b);
void avdl_vec4_CrossVec3(struct avdl_vec4 *a, struct avdl_vec3 *b);
float avdl_vec4_Distance(struct avdl_vec4 *a, struct avdl_vec4 *b);

void avdl_vec4_Print(struct avdl_vec4 *);

void avdl_vec4_Normalise(struct avdl_vec4 *o);
void avdl_vec4_Normalise3(struct avdl_vec4 *o);
float avdl_vec4_Magnitude(struct avdl_vec4 *o);

void avdl_vec4_Invert(struct avdl_vec4 *o);

// Quest 2 tests

#if defined(AVDL_QUEST2)
extern struct dd_matrix dd_cam_controllers[];
extern int dd_cam_controller_active[];
extern struct avdl_vec4 dd_cam_controllers_position[];
extern struct avdl_vec4 dd_cam_controllers_direction[];
#endif
void dd_matrix_setControllerMatrix(int controllerIndex, struct dd_matrix *m);
struct dd_matrix *dd_matrix_getControllerMatrix(int controllerIndex);
void dd_matrix_applyControllerMatrix(int controllerIndex);
int dd_matrix_hasVisibleControllers();
int dd_matrix_isControllerVisible(int index);
void dd_matrix_setControllerVisible(int index, int state);
struct avdl_vec4 *dd_matrix_getControllerPosition(int index);
struct avdl_vec4 *dd_matrix_getControllerDirection(int index);

void dd_matrix_quaternion_to_rotation_matrix(struct avdl_vec4 *q, struct dd_matrix *output);

#ifdef __cplusplus
}
#endif

#endif
