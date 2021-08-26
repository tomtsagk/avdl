#include "dd_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float dd_math_rad2dec(float rad) { return rad *180 /3.14; }
float dd_math_dec2rad(float dec) { return dec *3.14 /180; }
int dd_math_rand(int to) { return rand() %to; }
float dd_math_randf(float to) { return ((float) rand() /(float) RAND_MAX) *to; }

float dd_math_tan(float val) {return tan(val);}

float dd_math_ease_catmullrom(float t, float p0, float p1, float p2, float p3) {
	return 0.5 *(
		(2 *p1) +
		(-p0 +p2) *t +
		(2*p0 -5*p1 +4*p2 -p3) *dd_math_pow(t, 2) +
		(-p0 +3*p1 -3*p2 +p3) *dd_math_pow(t, 3)
	);
}

void dd_math_ease_catmullrom2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1, struct dd_vec2 p2, struct dd_vec2 p3) {
	point->x = dd_math_ease_catmullrom(t, p0.x, p1.x, p2.x, p3.x);
	point->y = dd_math_ease_catmullrom(t, p0.y, p1.y, p2.y, p3.y);
}

float dd_math_ease_linear(float t, float p0, float p1) {
	return p0 +(p1 -p0) *t;
}

void dd_math_ease_linear2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1) {
	point->x = dd_math_ease_linear(t, p0.x, p1.x);
	point->y = dd_math_ease_linear(t, p0.y, p1.y);
}

float dd_math_ease_bezier(float t, float p0, float p1, float p2) {
	return dd_math_ease_linear(t,
		dd_math_ease_linear(t, p0, p1),
		dd_math_ease_linear(t, p1, p2)
	);
}

void dd_math_ease_bezier2d(struct dd_vec2 *point, float t, struct dd_vec2 p0, struct dd_vec2 p1, struct dd_vec2 p2) {
}

float dd_math_dot2(struct dd_vec2 *v1, struct dd_vec2 *v2) {
	return v1->x *v2->x +v1->y *v2->y;
}

float dd_math_dot3(struct dd_vec3 *v1, struct dd_vec3 *v2) {
	return v1->x *v2->x +v1->y *v2->y +v1->z *v2->z;
}
