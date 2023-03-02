#include "avdl_string.h"
#include "avdl_log.h"
#include <string.h>
#include <stdarg.h>

void avdl_string_create(struct avdl_string *o, int maxCharacters) {

	// add one to maximum characters, to include the terminating null
	maxCharacters++;

	// temporarily have a minimum of 3 characters as max
	maxCharacters = maxCharacters > 3 ? maxCharacters : 3;

	o->errorCode = 0;
	o->maxCharacters = maxCharacters;
	dd_da_init(&o->string, sizeof(char));
	dd_da_push(&o->string, "\0");
}

void avdl_string_cat(struct avdl_string *o, const char *stringToCatenate) {

	// string is in error mode - do nothing
	if (o->errorCode) {
		return;
	}

	// maximum characters reached - error
	if (o->string.elements +strlen(stringToCatenate) > o->maxCharacters) {
		o->errorCode = 1;
		return;
	}
	dd_da_add(&o->string, stringToCatenate, strlen(stringToCatenate), -2);
}

int avdl_string_isValid(struct avdl_string *o) {
	return !o->errorCode;
}

char *avdl_string_getError(struct avdl_string *o) {
	if (o->errorCode == 1) {
		return "maximum number of characters reached";
	}
	else {
		return "unknown error";
	}
}

char *avdl_string_toCharPtr(struct avdl_string *o) {

	if (o->errorCode) {
		return "";
	}

	return o->string.array;
}

void avdl_string_clean(struct avdl_string *o) {

	if (o->errorCode == -1 ) {
		avdl_log_error("string cleaned twice");
		return;
	}

	dd_da_free(&o->string);
	o->errorCode = -1;
}

int avdl_string_endsIn(struct avdl_string *o, const char *endingString) {

	if (o->errorCode) {
		return 0;
	}

	// ending string bigger than source string - source string does not end with it
	if (strlen(endingString) >= o->string.elements) {
		return 0;
	}

	return strcmp(((char *)o->string.array) +o->string.elements -1 -strlen(endingString), endingString) == 0;
}

void avdl_string_replaceEnding(struct avdl_string *o, const char *fromEnding, const char *toEnding) {

	if (o->errorCode) {
		return;
	}

	// doesn't end in expected strings - do nothing
	if ( !avdl_string_endsIn(o, fromEnding) ) {
		return;
	}

	int position = o->string.elements -1 -strlen(fromEnding);
	dd_da_remove(&o->string, strlen(fromEnding), position);
	dd_da_add(&o->string, toEnding, strlen(toEnding), position);

}
