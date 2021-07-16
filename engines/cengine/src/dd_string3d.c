#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_cengine.h"

static struct dd_mesh letter[26];
static struct dd_mesh number[26];
extern GLuint fontProgram;
extern GLuint defaultProgram;

static int isActive = 0;
static const char *fontname = 0;
void dd_string3d_activate(const char *src) {
	isActive = 1;
	fontname = src;
}

int dd_string3d_isActive() {
	return isActive;
}

extern struct dd_matrix matPerspective;

static void dd_string3d_drawLetter(struct dd_string3d *font, struct dd_mesh *o) {
	int previousProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
	glUseProgram(fontProgram);
	GLuint MatrixID = glGetUniformLocation(fontProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, o->v);

	GLint loc = glGetUniformLocation(fontProgram, "colorFront");
	if (loc != -1)
	{
		glUniform3f(loc, font->colorFront[0], font->colorFront[1], font->colorFront[2]);
	}

	loc = glGetUniformLocation(fontProgram, "colorBack");
	if (loc != -1)
	{
		glUniform3f(loc, font->colorBack[0], font->colorBack[1], font->colorBack[2]);
	}

	glDrawArrays(GL_TRIANGLES, 0, o->vcount);

	glDisableVertexAttribArray(0);
	glUseProgram(previousProgram);
}

void dd_string3d_init() {

	// Make sure fontname is within the limits
	if (strlen(fontname) >= 50) {
		dd_log("avdl: string3d: font path should have less than 50 characters");
		return;
	}

	// Prepare buffer that will swap the characters to load the files
	char buffer[50];
	strcpy(buffer, fontname);
	char *special = buffer;

	// Find the special (?) character
	while (special[0] != '?') {
		special++;
		if (special -buffer >= strlen(buffer)) {
			dd_log("avdl: string3d: no special (?) character found");
			return;
		}
	}

	// Load letters
	for (int i = 'a'; i <= 'z'; i++) {
		special[0] = i;
		dd_mesh_create(&letter[i-'a']);
		letter[i-'a'].load(&letter[i-'a'], buffer);
	}

	// Load numbers
	for (int i = '0'; i <= '9'; i++) {
		special[0] = i;
		dd_mesh_create(&number[i-'0']);
		number[i-'0'].load(&number[i-'0'], buffer);
	}

} // string3d init

void dd_string3d_create(struct dd_string3d *o) {
	o->align = DD_STRING3D_ALIGN_LEFT;
	o->colorFront[0] = 1.0;
	o->colorFront[1] = 1.0;
	o->colorFront[2] = 1.0;
	o->colorBack[0] = 0.0;
	o->colorBack[1] = 0.0;
	o->colorBack[2] = 0.0;

	o->setAlign = dd_string3d_setAlign;
	o->setColorFront = dd_string3d_setColorFront;
	o->setColorBack = dd_string3d_setColorBack;
	o->clean = dd_string3d_clean;
	o->draw = dd_string3d_draw;
	o->drawInt = dd_string3d_drawInt;
	o->drawLimit = dd_string3d_drawLimit;

}

void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al) {
	o->align = al;
}

void dd_string3d_draw(struct dd_string3d *o, const char* text) {
	dd_string3d_drawLimit(o, text, 0);
}

void dd_string3d_drawInt(struct dd_string3d *o, int num) {
	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	dd_string3d_draw(o, numberString);
}

void dd_string3d_drawLimit(struct dd_string3d *o, const char* text, int limit) {

	if (limit <= 0) {
		limit = strlen(text);
	}

	int lines;
	lines = 1 +(strlen(text) /limit);
	if (strlen(text) %limit == 0) {
		lines--;
	}

	//const char *currentText = text;
	for (int i = 0; i < lines; i++) {
		dd_matrix_push();

		int lineWidth = dd_math_min(strlen(text), limit);
		dd_translatef(0, -1 *i, 0);

		switch (o->align) {
		case DD_STRING3D_ALIGN_LEFT:
			break;
		case DD_STRING3D_ALIGN_CENTER:
			dd_translatef(-(lineWidth *0.5)/2 +0.25, 0, 0);
			break;
		case DD_STRING3D_ALIGN_RIGHT:
			dd_translatef(-(lineWidth *0.5), 0, 0);
			break;
		}

		int j = 0;
		const char *currentLetter = text +(i*limit);
		while (currentLetter[0] != '\0' && j < limit) {

			if (currentLetter[0] >= 'A' && currentLetter[0] <= 'Z') {
				dd_string3d_drawLetter(o, &letter[currentLetter[0]-'A']);
			}
			else
			// Draw letter
			if (currentLetter[0] >= 'a' && currentLetter[0] <= 'z') {
				dd_string3d_drawLetter(o, &letter[currentLetter[0]-'a']);
			}
			else
			// Draw number
			if (currentLetter[0] >= '0' && currentLetter[0] <= '9') {
				dd_string3d_drawLetter(o, &number[currentLetter[0]-'0']);
			}
			dd_translatef(0.5, 0, 0);
			currentLetter++;
			j++;
		}
		dd_matrix_pop();
	}
}

void dd_string3d_clean(struct dd_string3d *o) {
}

void dd_string3d_setColorFront(struct dd_string3d *o, float r, float g, float b) {
	o->colorFront[0] = r;
	o->colorFront[1] = g;
	o->colorFront[2] = b;
}

void dd_string3d_setColorBack(struct dd_string3d *o, float r, float g, float b) {
	o->colorBack[0] = r;
	o->colorBack[1] = g;
	o->colorBack[2] = b;
}
