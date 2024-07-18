#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "avdl_commands.h"
#include "avdl_variable_type.h"

// native keywords
const char *keywords[] = {
	"echo",
	"log",
	"log_error",
	"def",

	"=",
	"+",
	"-",
	"*",
	"/",
	"%",
	">=",
	"==",
	"<=",
	"&&",
	"||",
	"<",
	">",
	"+=",
	"-=",
	"*=",
	"/=",

	"group",
	"struct",
	"class",
	"class_function",
	"function",
	"return",

	"if",
	"for",
	"asset",
	"savefile",

	"extern",
	"multistring",
	"unicode",
	"include",
};
unsigned int keywords_total = sizeof(keywords) /sizeof(char *);

int agc_commands_isNative(const char *cmdname) {
	for (int i = 0; i < keywords_total; i++) {
		if (strcmp(keywords[i], cmdname) == 0) {
			return 1;
		}
	}

	// primitive types are all commands
	return dd_variable_type_isPrimitiveType(cmdname);
}
