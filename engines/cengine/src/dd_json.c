#include "dd_json.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "dd_log.h"

#include <stdlib.h>
#include <wchar.h>
#include <stddef.h>

void dd_json_init(struct dd_json_object *o, char *json_string, int size) {
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

void dd_json_initFile(struct dd_json_object *o, char *filename) {
	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	o->file2 = AAssetManager_open(aassetManager, filename, AASSET_MODE_UNKNOWN);
	if (!o->file2)
	{
		dd_log("load_ply: error opening file: %s: %s", filename, "unknown error");
		return;
	}
	o->str2 = AAsset_getBuffer(o->file2);
	o->current = o->str2;
	#else
	o->file = fopen(filename, "r");
	if (!o->file) {
		dd_log("avdl: error opening json file '%s': %s",
			filename, strerror(errno)
		);
	}
	#endif
	o->hasKey = 0;
}

void dd_json_next(struct dd_json_object *o) {

	#ifdef AVDL_DIRECT3D11
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	if (o->file2) {

		// ignore whitespace
		while (o->current[0] == ' ' || o->current[0] == '\t' || o->current[0] == '\r' || o->current[0] == '\n') {
			o->current++;
		}

		// grab first character
		char nextChar = o->current[0];
		o->current++;

		char *st;
		switch (nextChar) {
			// skip commas
			case ',':
				dd_json_next(o);
				break;
			case ':':
				o->hasKey = 1;
				dd_json_next(o);
				break;
			// start of array
			case '[':
				o->token = DD_JSON_ARRAY_START;
				o->length = 1;
				break;
			// end of array
			case ']':
				o->token = DD_JSON_ARRAY_END;
				o->length = 1;
				break;
			// start of object
			case '{':
				o->token = DD_JSON_OBJECT_START;
				o->length = 1;
				break;
			// end of object
			case '}':
				o->token = DD_JSON_OBJECT_END;
				o->length = 1;
				break;
			// key or string
			case '"':
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
					o->token = DD_JSON_KEY;
				}
				// string
				else {
					o->token = DD_JSON_STRING;
					o->hasKey = 0;
				}
				break;
			// number or unknown
			default:
				dd_log("number or unknown: %c %d %s", nextChar, nextChar, o->current);
				/*
				// number
				if (nextChar >= '0' && nextChar <= '9') {
					int num;
					fscanf(o->file, "%d", &num);
					o->token = DD_JSON_INT;
					dd_log("not parsing numbers for now");
				}
				// unknown
				else {
					printf("unable to parse\n");
					goto end_of_buffer;
				}
				*/
				break;
		}

	}
	#else
	if (o->file) {

		// ignore whitespace
		fscanf(o->file, "%200[ \t\n\r]", o->buffer);

		// grab first character
		char nextChar;
		fscanf(o->file, "%c", &nextChar);

		switch (nextChar) {
			// skip commas
			case ',':
				dd_json_next(o);
				break;
			case ':':
				o->hasKey = 1;
				dd_json_next(o);
				break;
			// start of array
			case '[':
				o->token = DD_JSON_ARRAY_START;
				o->length = 1;
				break;
			// end of array
			case ']':
				o->token = DD_JSON_ARRAY_END;
				o->length = 1;
				break;
			// start of object
			case '{':
				o->token = DD_JSON_OBJECT_START;
				o->length = 1;
				break;
			// end of object
			case '}':
				o->token = DD_JSON_OBJECT_END;
				o->length = 1;
				break;
			// key or string
			case '"':

				// scan key-string
				fscanf(o->file, "%200[^\"]", o->buffer);
				fscanf(o->file, "%*c");

				// ignore whitespace
				fscanf(o->file, "%*[ \t\n\r]");

				// key
				if (!o->hasKey) {
					o->token = DD_JSON_KEY;
				}
				// string
				else {
					o->token = DD_JSON_STRING;
					o->hasKey = 0;
				}
				break;
			// number or unknown
			default:
				dd_log("number or unknown");
				/*
				// number
				if (nextChar >= '0' && nextChar <= '9') {
					int num;
					fscanf(o->file, "%d", &num);
					o->token = DD_JSON_INT;
					dd_log("not parsing numbers for now");
				}
				// unknown
				else {
					printf("unable to parse\n");
					goto end_of_buffer;
				}
				*/
				break;
		}

	}

	#endif
}

enum DD_JSON_TOKEN dd_json_getToken(struct dd_json_object *o) {
	return o->token;
}

char *dd_json_getTokenString(struct dd_json_object *o) {
	return o->buffer;
}

int dd_json_getTokenSize(struct dd_json_object *o) {
	return o->length;
}

void dd_json_deinit(struct dd_json_object *o) {
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
