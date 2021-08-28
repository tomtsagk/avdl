#ifndef DD_MATH_H
#define DD_MATH_H

#include "dd_vec2.h"
#include "dd_vec3.h"
#include <math.h>
#include <stdlib.h>

// Random
int   dd_math_rand (int   to);
float dd_math_randf(float to);

/*
 * Eases
 *
 * Linear
 * Bezier
 * Catmull-Rom
 *
 * The 2D versions do the same on a vector2d
 *
 */
float dd_math_ease_linear(float t, float p0, float p1);
void dd_math_ease_linear2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1);

float dd_math_ease_bezier(float t, float p0, float p1, float p2);
void dd_math_ease_bezier2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1, struct dd_vec2 p2);

float dd_math_ease_catmullrom(float t, float p0, float p1, float p2, float p3);
void dd_math_ease_catmullrom2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1, struct dd_vec2 p2, struct dd_vec2 p3);

// operations
#define dd_math_pow(x, times) pow(x, times)
#define dd_math_abs(val) fabs((float)val)
#define dd_math_sqrt(val) sqrt(val)
#define dd_math_sin(val) sinf(val)
#define dd_math_cos(val) cosf(val)
#define dd_math_tan(val) tanf(val)
#define dd_math_asin(val) asinf(val)
#define dd_math_acos(val) acosf(val)
#define dd_math_atan(val) atanf(val)
#define dd_math_floor(val) floorf(val)
#define dd_math_ceil(val) ceilf(val)

float dd_math_tan(float val);

#define dd_math_min(a, b) (a > b ? b : a)
#define dd_math_max(a, b) (a > b ? a : b)
#define dd_math_minf(a, b) (a > b ? b : a)
#define dd_math_maxf(a, b) (a > b ? a : b)

float dd_math_rad2dec(float rad);
float dd_math_dec2rad(float dec);

// vector math
float dd_math_dot2(struct dd_vec2 *v1, struct dd_vec2 *v2);
float dd_math_dot3(struct dd_vec3 *v1, struct dd_vec3 *v2);


#endif
