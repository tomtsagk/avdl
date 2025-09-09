#ifndef AVDL_STRING_H
#define AVDL_STRING_H

#include "shared/avdl_dynamic_array.h"

struct avdl_string {
	int maxCharacters;
	struct avdl_dynamic_array string;
	int errorCode;
	int errorCharacters;

	void (*clean)(struct avdl_string *);
};

void avdl_string_create(struct avdl_string *o);
void avdl_string_SetMaxCharacters(struct avdl_string *o, int maxCharacters);
void avdl_string_cat(struct avdl_string *o, const char *stringToCatenate);
void avdl_string_ncat(struct avdl_string *o, const char *stringToCatenate, int size);
int avdl_string_isValid(struct avdl_string *o);
char *avdl_string_getError(struct avdl_string *o);
char *avdl_string_toCharPtr(struct avdl_string *o);
void avdl_string_clean(struct avdl_string *o);

int avdl_string_endsIn(struct avdl_string *o, const char *endingString);
void avdl_string_replaceEnding(struct avdl_string *o, const char *fromEnding, const char *toEnding);
int avdl_string_endsInInt(struct avdl_string *o);
int avdl_string_incrementEndingInt(struct avdl_string *o);

void avdl_string_copy(struct avdl_string *o, struct avdl_string *target);

void avdl_string_empty(struct avdl_string *o);

#endif
