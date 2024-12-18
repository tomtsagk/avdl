#include "avdl_json.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "avdl_log.h"

#include <stdlib.h>
#include <wchar.h>
#include <stddef.h>

void avdl_json_init(struct avdl_json_object *o, char *json_string, int size) {
	o->str = json_string;
	o->size = size;

	o->start = o->str;
	o->end = o->start;
	o->length = 0;

	o->buffer[0] = '\0';

	o->file = 0;

	o->hasKey = 0;
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
extern AAssetManager *aassetManager;
#endif

void avdl_json_initFile(struct avdl_json_object *o, char *filename) {

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	o->file2 = AAssetManager_open(aassetManager, filename, AASSET_MODE_UNKNOWN);
	if (!o->file2)
	{
		avdl_log_error("json: error opening file: %s: %s", filename, "unknown error");
		return;
	}
	o->str2 = AAsset_getBuffer(o->file2);
	o->current = o->str2;
	#else

	#if defined( AVDL_DIRECT3D11 )
	fopen_s(&o->file, filename, "r");
	#else
	o->file = fopen(filename, "r");
	#endif
	if (!o->file) {
		avdl_log_error("error opening json file '%s': %s", filename,
			#if defined( AVDL_DIRECT3D11 )
			"error"
			#else
			strerror(errno)
			#endif
		);
	}

	fseek(o->file, 0L, SEEK_END);
	size_t size = ftell(o->file);
	rewind(o->file);

	o->str = malloc((size +1) *sizeof(char));
	while (fscanf(o->file, "%1024c", o->str) > 0) {
	}
	o->str[size] = '\0';
	//avdl_log("json file: %s", o->str);
	o->current = o->str;

	#endif
	o->hasKey = 0;
	o->isNumber = 0;
	o->isFloat = 0;
}

void avdl_json_next(struct avdl_json_object *o) {

	int hasFile;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	hasFile = o->file2 != 0;
	#else
	hasFile = o->file != 0;
	#endif

	if (hasFile) {

		// ignore whitespace
		while (o->current[0] == ' ' || o->current[0] == '\t' || o->current[0] == '\r' || o->current[0] == '\n') {
			o->current++;
		}

		o->isNumber = 0;
		o->isFloat = 0;

		// grab first character
		char nextChar = o->current[0];

		char *st;
		switch (nextChar) {
			// skip commas
			case ',':
				o->current++;
				avdl_json_next(o);
				break;
			case ':':
				o->current++;
				o->hasKey = 1;
				avdl_json_next(o);
				break;
			// start of array
			case '[':
				o->current++;
				o->token = AVDL_JSON_ARRAY_START;
				o->length = 1;
				o->hasKey = 0;
				break;
			// end of array
			case ']':
				o->current++;
				o->token = AVDL_JSON_ARRAY_END;
				o->length = 1;
				break;
			// start of object
			case '{':
				o->current++;
				o->token = AVDL_JSON_OBJECT_START;
				o->length = 1;
				o->hasKey = 0;
				break;
			// end of object
			case '}':
				o->current++;
				o->token = AVDL_JSON_OBJECT_END;
				o->length = 1;
				break;
			// key or string
			case '"':
				o->current++;
				st = o->current;
				while (st[0] != '"') {
					st++;
				}
				strncpy(o->buffer, o->current, st -o->current);
				o->buffer[st -o->current] = '\0';
				o->current = st +1;

				// ignore whitespace
				while (o->current[0] == ' ' || o->current[0] == '\t' || o->current[0] == '\r' || o->current[0] == '\n') {
					o->current++;
				}

				// key
				if (!o->hasKey) {
					o->token = AVDL_JSON_KEY;
				}
				// string
				else {
					o->token = AVDL_JSON_STRING;
					o->hasKey = 0;
				}
				break;
			// number or unknown
			default:

				// check float
				int has_comma = 0;
				o->isFloat = 0;
				char *end = o->current;
				while ((end[0] >= '0' && end[0] <= '9') || end[0] == '.') {
					if (end[0] == '.') {
						if (has_comma) {
							avdl_log("json: float has more than one commas");
						}
						has_comma = 1;
					}
					else {
						// has digit after comma - is float
						if (has_comma) {
							o->isFloat = 1;
						}
					}
					end++;
				}
				if (o->isFloat) {
					o->token = AVDL_JSON_FLOAT;
					o->numberf = atof(o->current);
					o->current = end;
					o->hasKey = 0;
					break;
				}

				// check int
				o->isNumber = 0;
				end = o->current;
				while (end[0] >= '0' && end[0] <= '9') {
					end++;
				}
				if (end > o->current) {
					o->isNumber = 1;
				}
				if (o->isNumber) {
					o->token = AVDL_JSON_INT;
					o->number = atoi(o->current);
					o->current = end;
					o->hasKey = 0;
					break;
				}

				avdl_log("number or unknown: %c %d %s", nextChar, nextChar, o->current);
				o->current++;
				break;
		}

	}
}

enum AVDL_JSON_TOKEN avdl_json_getToken(struct avdl_json_object *o) {
	return o->token;
}

char *avdl_json_getTokenString(struct avdl_json_object *o) {
	return o->buffer;
}

int avdl_json_getTokenNumber(struct avdl_json_object *o) {
	return o->number;
}

float avdl_json_getTokenFloat(struct avdl_json_object *o) {
	return o->numberf;
}

int avdl_json_getTokenSize(struct avdl_json_object *o) {
	return o->length;
}

void avdl_json_deinit(struct avdl_json_object *o) {
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	if (o->file2) {
		AAsset_close(o->file2);
		o->file2 = 0;
	}
	#else
	if (o->file) {
		fclose(o->file);
		o->file = 0;
	}
	#endif

	o->str = 0;
	o->size = 0;

	o->start = 0;
	o->end = 0;
	o->length = 0;

	o->buffer[0] = '\0';
}
