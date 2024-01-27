#ifndef AVDL_STRING_H
#define AVDL_STRING_H

#include "avdl_dynamic_array.h"

struct avdl_string {
	int maxCharacters;
	struct avdl_dynamic_array string;
	int errorCode;
	int errorCharacters;
};

void avdl_string_create(struct avdl_string *o, int maxCharacters);
void avdl_string_cat(struct avdl_string *o, const char *stringToCatenate);
int avdl_string_isValid(struct avdl_string *o);
char *avdl_string_getError(struct avdl_string *o);
char *avdl_string_toCharPtr(struct avdl_string *o);
void avdl_string_clean(struct avdl_string *o);

int avdl_string_endsIn(struct avdl_string *o, const char *endingString);
void avdl_string_replaceEnding(struct avdl_string *o, const char *fromEnding, const char *toEnding);

#endif
