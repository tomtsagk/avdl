#ifndef DD_VEC3_H
#define DD_VEC3_H

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_vec3 {
	float x, y, z;
};

void avdl_vec3_create(struct avdl_vec3 *o);

void avdl_vec3_Set(struct avdl_vec3 *o, struct avdl_vec3 *src);
void avdl_vec3_Setf(struct avdl_vec3 *o, float x, float y, float z);

// math
void avdl_vec3_Add(struct avdl_vec3 *o, struct avdl_vec3 *a, struct avdl_vec3 *b);
void avdl_vec3_Addf(struct avdl_vec3 *o1, float x, float y, float z);

void avdl_vec3_Subtract(struct avdl_vec3 *o, struct avdl_vec3 *a, struct avdl_vec3 *b);
void avdl_vec3_Subtractf(struct avdl_vec3 *o, float x, float y, float z);

void avdl_vec3_Multiply(struct avdl_vec3 *o, struct avdl_vec3 *a, struct avdl_vec3 *b);
void avdl_vec3_Multiplyf(struct avdl_vec3 *o, float x, float y, float z);

void avdl_vec3_Divide(struct avdl_vec3 *o, struct avdl_vec3 *a, struct avdl_vec3 *b);
void avdl_vec3_Dividef(struct avdl_vec3 *o, float x, float y, float z);

// getters
float avdl_vec3_X(struct avdl_vec3 *o);
float avdl_vec3_Y(struct avdl_vec3 *o);
float avdl_vec3_Z(struct avdl_vec3 *o);

void avdl_vec3_clean(struct avdl_vec3 *o);

// more math
void avdl_vec3_Cross(struct avdl_vec3 *o, struct avdl_vec3 *v1, struct avdl_vec3 *v2);
float avdl_vec3_Dot(struct avdl_vec3 *a, struct avdl_vec3 *b);
void avdl_vec3_Normalise(struct avdl_vec3 *o);
float avdl_vec3_Magnitude(struct avdl_vec3 *o);

float avdl_vec3_RotateX(struct avdl_vec3 *o, float rad);
float avdl_vec3_RotateY(struct avdl_vec3 *o, float rad);
float avdl_vec3_RotateZ(struct avdl_vec3 *o, float rad);

void avdl_vec3_Print(struct avdl_vec3 *);

/*
// inline functions
const char *avdl_vec3_inline_functions[] = {
	"InlineTest",
};
unsigned int avdl_vec3_inline_functions_count = sizeof(avdl_vec3_inline_functions) /sizeof(char *);
*/

#ifdef __cplusplus
}
#endif

#endif
