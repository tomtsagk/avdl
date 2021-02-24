#include <stdio.h>
#include <stdlib.h>
#include "dd_meshRising.h"
#include <string.h>
#include <sys/time.h>
#include <dd_opengl.h>
#include "dd_log.h"

extern GLuint defaultProgram;
extern GLuint risingProgram;

// constructor
void dd_meshRising_create(struct dd_meshRising *m) {
	dd_meshColour_create(&m->parent);
	m->animationCurrent = 1;
	m->animationMax = 1;
	m->parent.parent.draw = (void (*)(struct dd_mesh *)) dd_meshRising_draw;
	m->set_animation_max = (void (*)(struct dd_mesh *, float)) dd_meshRising_set_animation_max;
	m->set_animation_current = (void (*)(struct dd_mesh *, float)) dd_meshRising_set_animation_current;
}
extern struct dd_matrix matPerspective;

/* draw the mesh itself */
void dd_meshRising_draw(struct dd_meshRising *m) {
	glUseProgram(risingProgram);
	GLuint MatrixID2 = glGetUniformLocation(risingProgram, "matrixProjection");
	glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, (float *)&matPerspective);
	GLuint MatrixID = glGetUniformLocation(risingProgram, "matrix");
	glUniformMatrix4fv(MatrixID , 1, GL_FALSE, (float *)dd_matrix_globalGet());

	GLuint animLoc = glGetUniformLocation(risingProgram, "animationCurrent");
	glUniform1f(animLoc, m->animationCurrent);
	dd_meshColour_draw((struct dd_meshColour*) m);

	glUseProgram(defaultProgram);
}

void dd_meshRising_set_animation_max(struct dd_meshRising *m, float val) {
	m->animationMax = val;
}

void dd_meshRising_set_animation_current(struct dd_meshRising *m, float val) {
	m->animationCurrent = val;
}
