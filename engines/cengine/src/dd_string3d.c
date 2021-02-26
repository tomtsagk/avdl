#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avdl_cengine.h"

static struct dd_mesh letter[26];
static struct dd_mesh number[26];
#if DD_PLATFORM_ANDROID
static const char *fontShader_vertexES;
static const char *fontShader_vertexES101;
static const char *fontShader_fragmentES;
static const char *fontShader_fragmentES101;
#elif DD_PLATFORM_NATIVE
/*
static const char *fontShader_vertex;
static const char *fontShader_vertex110;
static const char *fontShader_fragment;
static const char *fontShader_fragment110;
*/
#endif
static GLuint fontProgram;
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
	/*
	glUseProgram(fontProgram);
	GLuint MatrixID2 = glGetUniformLocation(fontProgram, "matrixProjection");
	glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, (float *)&matPerspective);
	GLuint MatrixID = glGetUniformLocation(fontProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());
	*/
	GLuint MatrixID = glGetUniformLocation(defaultProgram, "matrix");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float *)dd_matrix_globalGet());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, o->v);

	GLint loc = glGetUniformLocation(fontProgram, "colorFrontR");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorFront[0]);
	}
	loc = glGetUniformLocation(fontProgram, "colorFrontG");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorFront[1]);
	}
	loc = glGetUniformLocation(fontProgram, "colorFrontB");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorFront[2]);
	}

	loc = glGetUniformLocation(fontProgram, "colorBackR");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorBack[0]);
	}
	loc = glGetUniformLocation(fontProgram, "colorBackG");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorBack[1]);
	}
	loc = glGetUniformLocation(fontProgram, "colorBackB");
	if (loc != -1)
	{
		glUniform1f(loc, font->colorBack[2]);
	}

	glDrawArrays(GL_TRIANGLES, 0, o->vcount);

	glDisableVertexAttribArray(0);
	//glUseProgram(defaultProgram);
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

	#if DD_PLATFORM_ANDROID
	fontProgram = 0;
	/*
	fontProgram = avdl_loadProgram(fontShader_vertexES, fontShader_fragmentES);
	if (!fontProgram) {
		fontProgram = load_program(fontShader_vertexES101, fontShader_fragmentES101);
		if (!fontProgram) {
			dd_log("avdl: dd_string3d: error loading shaders");
			exit(-1);
		}
		else {
			dd_log("avdl: falling back to glsl version es 101");
			dd_log("");
		}
	}
	*/

	#elif DD_PLATFORM_NATIVE

	fontProgram = 0;
	/*
	// Prepare shader for drawing letters
	fontProgram = load_program(fontShader_vertex, fontShader_fragment);
	if (!fontProgram) {
		fontProgram = load_program(fontShader_vertex110, fontShader_fragment110);
		if (!fontProgram) {
			dd_log("avdl: dd_string3d: error loading font shaders");
			exit(-1);
		}
		else {
			dd_log("avdl: dd_string3d: falling back to version 1.10");
			dd_log("");
		}
	}
	*/

	#endif

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

}

void dd_string3d_setAlign(struct dd_string3d *o, enum dd_string3d_align al) {
	o->align = al;
}

void dd_string3d_draw(struct dd_string3d *o, const char* text) {
	dd_matrix_push();

	switch (o->align) {
	case DD_STRING3D_ALIGN_LEFT:
		break;
	case DD_STRING3D_ALIGN_CENTER:
		dd_translatef(-(strlen(text) *0.5)/2 +0.25, 0, 0);
		break;
	case DD_STRING3D_ALIGN_RIGHT:
		dd_translatef(-(strlen(text) *0.5), 0, 0);
		break;
	}

	const char *currentLetter = text;
	while (currentLetter[0] != '\0') {

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
	}
	dd_matrix_pop();
}

void dd_string3d_drawInt(struct dd_string3d *o, int num) {
	char numberString[11];
	snprintf(numberString, 11, "%d", num);
	dd_string3d_draw(o, numberString);
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

#if DD_PLATFORM_ANDROID
static const char *fontShader_vertexES =
"#version 310 es\n"
"in vec4 position;\n"
"uniform float colorFrontR;\n"
"uniform float colorFrontG;\n"
"uniform float colorFrontB;\n"
"uniform float colorBackR;\n"
"uniform float colorBackG;\n"
"uniform float colorBackB;\n"
"uniform mat4 matrix;\n"
"uniform mat4 matrixProjection;\n"
"out vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"
"	if (position.z > 0.0) {\n"
"		outColour = vec4(colorFrontR, colorFrontG, colorFrontB, 1);\n"
"	}\n"
"	else {\n"
"		outColour = vec4(colorBackR, colorBackG, colorBackB, 1);\n"
"	}\n"
"}\n"
;
static const char *fontShader_vertexES101 =
"#version 101\n"
"attribute vec4 position;\n"
"uniform float colorFrontR;\n"
"uniform float colorFrontG;\n"
"uniform float colorFrontB;\n"
"uniform float colorBackR;\n"
"uniform float colorBackG;\n"
"uniform float colorBackB;\n"
"uniform mat4 matrix;\n"
"uniform mat4 matrixProjection;\n"
"varying vec4 outColour;\n"
"void main() {\n"
"	gl_Position = matrixProjection *matrix *position;\n"
"	if (position.z > 0.0) {\n"
"		outColour = vec4(colorFrontR, colorFrontG, colorFrontB, 1);\n"
"	}\n"
"	else {\n"
"		outColour = vec4(colorBackR, colorBackG, colorBackB, 1);\n"
"	}\n"
"}\n"
;
static const char *fontShader_fragmentES =
"#version 310 es\n"
"in highp vec4 outColour;\n"
"layout(location = 0) out highp vec4 frag_color;\n"
"void main() {\n"
"	frag_color = outColour;\n"
"}\n"
;

static const char *fontShader_fragmentES101 =
"#version 101\n"
"varying highp vec4 outColour;\n"
"void main() {\n"
"	gl_FragColor = outColour;\n"
"}\n"
;
#elif DD_PLATFORM_NATIVE
/*
static const char *fontShader_vertex =
"#version 130\n"
"in vec4 position;\n"
"uniform float colorFrontR;\n"
"uniform float colorFrontG;\n"
"uniform float colorFrontB;\n"
"uniform float colorBackR;\n"
"uniform float colorBackG;\n"
"uniform float colorBackB;\n"
"uniform mat4 matrix;\n"
"void main() {\n"
"	gl_Position = matrix *position;\n"
"	if (position.z > 0) {\n"
"		gl_FrontColor = vec4(colorFrontR, colorFrontG, colorFrontB, 1);\n"
"	}\n"
"	else {\n"
"		gl_FrontColor = vec4(colorBackR, colorBackG, colorBackB, 1);\n"
"	}\n"
"}\n"
;
static const char *fontShader_fragment =
"#version 130\n"
"void main() {\n"
"	gl_FragColor = gl_Color;\n"
"}\n"
;
static const char *fontShader_vertex110 =
"#version 110\n"
"attribute vec4 position;\n"
"uniform float colorFrontR;\n"
"uniform float colorFrontG;\n"
"uniform float colorFrontB;\n"
"uniform float colorBackR;\n"
"uniform float colorBackG;\n"
"uniform float colorBackB;\n"
"void main() {\n"
"	gl_Position = gl_ProjectionMatrix *gl_ModelViewMatrix *position;\n"
"	if (position.z > 0.0) {\n"
"		gl_FrontColor = vec4(colorFrontR, colorFrontG, colorFrontB, 1.0);\n"
"	}\n"
"	else {\n"
"		gl_FrontColor = vec4(colorBackR, colorBackG, colorBackB, 1.0);\n"
"	}\n"
"}\n"
;
static const char *fontShader_fragment110 =
"#version 110\n"
"void main() {\n"
"	gl_FragColor = gl_Color;\n"
"}\n"
;
*/
#endif
