#ifndef DD_JSON_H
#define DD_JSON_H

#include <stdio.h>

// Tokens
enum DD_JSON_TOKEN {

	// Objects
	DD_JSON_OBJECT_START,
	DD_JSON_OBJECT_END,

	DD_JSON_ARRAY_START,
	DD_JSON_ARRAY_END,

	// Data types
	DD_JSON_KEY,
	DD_JSON_STRING,
	DD_JSON_INT,

	// End of buffer
	DD_JSON_EOB,
};

#define DD_JSON_BUFFER_SIZE 255

struct dd_json_object {

	/*
	 * string parsing
	 */
	char buffer[DD_JSON_BUFFER_SIZE];
	char *str;
	int size;

	char *start;
	char *end;

	/*
	 * file parsing
	 */
	FILE *file;

	/*
	 * current token
	 */
	int length;
	enum DD_JSON_TOKEN token;

};

void dd_json_init(struct dd_json_object *o, char *json_string, int size);
void dd_json_initFile(struct dd_json_object *o, char *filename);

void dd_json_next(struct dd_json_object *o);
enum DD_JSON_TOKEN dd_json_getToken(struct dd_json_object *o);
char *dd_json_getTokenString(struct dd_json_object *o);
int dd_json_getTokenSize(struct dd_json_object *o);

void dd_json_deinit(struct dd_json_object *o);

#endif
