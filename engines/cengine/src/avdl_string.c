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
	o->errorCharacters = 0;
	o->maxCharacters = maxCharacters;
	dd_da_init(&o->string, sizeof(char));
	dd_da_push(&o->string, "\0");
}

void avdl_string_cat(struct avdl_string *o, const char *stringToCatenate) {

	o->errorCharacters += strlen(stringToCatenate);

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
	else
	if (o->errorCode == 0) {
		return "";
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
		avdl_logError("string cleaned twice");
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

void avdl_string_copy(struct avdl_string *o, struct avdl_string *target) {
	if (!target) {
		avdl_log("avdl_string_copy: given empty target");
		return;
	}
	avdl_string_clean(o);
	avdl_string_create(o, target->maxCharacters);
	avdl_string_cat(o, avdl_string_toCharPtr(target));
}

void avdl_string_empty(struct avdl_string *o) {
	dd_da_empty(&o->string);
	dd_da_push(&o->string, "\0");
	o->errorCharacters = 0;
	o->errorCode = 0;
}

int avdl_string_endsInInt(struct avdl_string *o) {
	if (o->errorCode) {
		return 0;
	}

	char *p = avdl_string_toCharPtr(o);
	int length = strlen(p);
	if (p[length-1] >= '0' && p[length-1] <= '9') {
		return 1;
	}

	return 0;
}

static int avdl_string_incrementIndexInt(struct avdl_string *o, int index) {
	char *p = avdl_string_toCharPtr(o);

	// not a digit
	if (p[index] < '0' || p[index] > '9') {
		return -1;
	}

	// increment digit
	for (int i = 0; i < 9; i++) {
		if (p[index] == ('0' +i)) {
			p[index]++;
			return 0;
		}
	}

	// increment a '9', needs special handling
	if (p[index] == '9') {

		p[index] = '0';

		// incremented a previous digit - all good
		if (avdl_string_incrementIndexInt(o, index-1) == 0) {
			return 0;
		}

		// no previous digit, set to '1' and add a new digit
		p[index] = '1';
		avdl_string_cat(o, "0");
		return 0;
	}
}

int avdl_string_incrementEndingInt(struct avdl_string *o) {
	if (o->errorCode) {
		return -1;
	}

	if (!avdl_string_endsInInt(o)) {
		avdl_log("string does not end in int");
		return -1;
	}

	char *p = avdl_string_toCharPtr(o);
	int length = strlen(p);
	char *lastdigit = p +(length -1);
	avdl_string_incrementIndexInt(o, length-1);

	return 0;
}
