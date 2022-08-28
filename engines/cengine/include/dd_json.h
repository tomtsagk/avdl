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

#define DD_JSON_BUFFER_SIZE 1000

struct dd_json_object {

	/*
	 * string parsing
	 */
	#if defined(WIN32) || defined(_WIN32)
	wchar_t bufferW[DD_JSON_BUFFER_SIZE];
	#endif
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

	int hasKey;

};

void dd_json_init(struct dd_json_object *o, char *json_string, int size);
#if defined(_WIN32) || defined(WIN32)
void dd_json_initFile(struct dd_json_object *o, wchar_t *filename);
#else
void dd_json_initFile(struct dd_json_object *o, char *filename);
#endif

void dd_json_next(struct dd_json_object *o);
enum DD_JSON_TOKEN dd_json_getToken(struct dd_json_object *o);
char *dd_json_getTokenString(struct dd_json_object *o);
int dd_json_getTokenSize(struct dd_json_object *o);

void dd_json_deinit(struct dd_json_object *o);

#endif
