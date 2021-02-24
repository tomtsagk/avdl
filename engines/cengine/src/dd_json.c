#include "dd_json.h"
#include <stdio.h>
#include <string.h>

void dd_json_init(struct dd_json_object *o, char *json_string, int size) {
	o->str = json_string;
	o->size = size;

	o->start = o->str;
	o->end = o->start;
	o->length = 0;

	o->buffer[0] = '\0';
}

void dd_json_next(struct dd_json_object *o) {

	// check for end of buffer
	if (o->end >= o->str +o->size) {
		end_of_buffer:
		//printf("end of buffer\n");
		o->start = o->str;
		o->end = o->start;
		o->length = 0;
		o->token = DD_JSON_EOB;
		return;
	}

	o->start = o->end;

	switch (o->start[0]) {
		// skip commas
		case ',':
			o->end++;
			dd_json_next(o);
			break;
		// start of array
		case '[':
			//printf("found start of array\n");
			o->end = o->start +1;
			o->token = DD_JSON_ARRAY_START;
			o->length = 1;
			break;
		// end of array
		case ']':
			//printf("found end of array\n");
			o->end = o->start +1;
			o->token = DD_JSON_ARRAY_END;
			o->length = 1;
			break;
		// start of object
		case '{':
			//printf("found object\n");
			o->end = o->start +1;
			o->token = DD_JSON_OBJECT_START;
			o->length = 1;
			break;
		// end of object
		case '}':
			//printf("found object end\n");
			o->end = o->start +1;
			o->token = DD_JSON_OBJECT_END;
			o->length = 1;
			break;
		// key or string
		case '"':
			o->start++;
			o->end = o->start;
			while (o->end[0] != '"') {
				o->end++;
				if (o->end >= o->str +o->size) {
					goto end_of_buffer;
				}
			}
			o->length = o->end -o->start;
			if (o->length >= DD_JSON_BUFFER_SIZE) {
				goto end_of_buffer;
			}
			strncpy(o->buffer, o->start, o->length);
			o->buffer[o->length] = '\0';
			o->end++;

			// key
			if (o->end < o->str +o->size && o->end[0] == ':') {
				//printf("found key\n");
				o->end++;
				o->token = DD_JSON_KEY;
			}
			// string
			else {
				//printf("found string\n");
				o->token = DD_JSON_STRING;
			}
			break;
		// number or unknown
		default:
			// number
			if (o->start[0] >= '0' && o->start[0] <= '9') {
				while (o->end[0] >= '0' && o->end[0] <= '9') {
					o->end++;
					if (o->end >= o->str +o->size) {
						goto end_of_buffer;
					}
				}
				o->length = o->end -o->start;
				if (o->length >= DD_JSON_BUFFER_SIZE) {
					goto end_of_buffer;
				}
				strncpy(o->buffer, o->start, o->length);
				o->buffer[o->length] = '\0';
				//printf("found int\n");
				o->token = DD_JSON_INT;
			}
			// unknown
			else {
				//printf("unable to parse\n");
				goto end_of_buffer;
			}
			break;
	}
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
