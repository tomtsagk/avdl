#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "avdl_commands.h"

// keywords for primitive data
const char *primitive_keywords[] = {
	"int",
	"float",
	"char",
	"string",
};
unsigned int primitive_keywords_count = sizeof(primitive_keywords) /sizeof(char *);

// native keywords
const char *keywords[] = {
	"echo",
	"log",
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

	"group",
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
	return 0;
}
