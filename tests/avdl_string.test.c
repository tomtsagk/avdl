#include "avdl_string.h"
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {

	struct avdl_string string;

	// check standard functionality
	avdl_string_create(&string, 5);
	avdl_string_cat(&string, "ab");
	assert(avdl_string_isValid(&string));
	assert(strcmp(avdl_string_getError(&string), "") == 0);
	assert(strcmp(avdl_string_toCharPtr(&string), "ab") == 0);

	avdl_string_cat(&string, "cd");
	assert(avdl_string_isValid(&string));
	assert(strcmp(avdl_string_getError(&string), "") == 0);
	assert(strcmp(avdl_string_toCharPtr(&string), "abcd") == 0);

	assert(avdl_string_endsIn(&string, "abcd"));
	assert(avdl_string_endsIn(&string, "bcd"));
	assert(avdl_string_endsIn(&string, "cd"));
	assert(avdl_string_endsIn(&string, "d"));
	assert(!avdl_string_endsIn(&string, "g"));

	avdl_string_replaceEnding(&string, "cd", "ge");
	assert(avdl_string_endsIn(&string, "abge"));
	assert(avdl_string_endsIn(&string, "bge"));
	assert(avdl_string_endsIn(&string, "ge"));
	assert(avdl_string_endsIn(&string, "e"));
	assert(!avdl_string_endsIn(&string, "d"));

	assert(!avdl_string_endsIn(&string, "too long string, it will never end in it"));

	avdl_string_replaceEnding(&string, "doesn't end in this", "won't replace with this");
	assert(avdl_string_endsIn(&string, "abge"));

	// check edge cases
	avdl_string_cat(&string, "too long string to fit");
	assert(!avdl_string_isValid(&string));
	assert(strcmp(avdl_string_getError(&string), "") != 0);
	avdl_string_cat(&string, "some other string");
	assert(strcmp(avdl_string_toCharPtr(&string), "") == 0);
	assert(!avdl_string_endsIn(&string, "it doesn't matter"));
	avdl_string_replaceEnding(&string, "ending one", "ending two");

	avdl_string_clean(&string);

	// second cleaning should be handled and ignored
	avdl_string_clean(&string);

	return 0;
}
