#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "dd_commands.h"
#include <ctype.h>

extern char buffer[];

static FILE *lex_file;
void lexer_prepare(const char *filename) {

	printf("lexing: %s\n", filename);

	int lineNumber = 1;

	lex_file = 0;
	lex_file = fopen(filename, "r");
	if (!lex_file) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
		exit(-1);
	}
}

void lexer_clean() {
	if (lex_file) {
		fclose(lex_file);
		lex_file = 0;
	}
}

static long lex_rewind_pos;
int lexer_getNextToken() {

	// clear characters that are not tokens (whitespace / comments)
	int allClear;
	do {
		allClear = 0;

		// new line
		allClear += fscanf(lex_file, "%1[\n]", buffer);
		if (allClear) {
			//printf("ignored new line\n");
			//lineNumber++;
		}

		// ignore whitespace
		allClear += fscanf(lex_file, "%100[ \t]*", buffer);
		//printf("ignored whitespace: %s\n", buffer);

		// ignore comments
		allClear += fscanf(lex_file, "%1[#]%*[^\n]*", buffer);
		//printf("ignored comment: %s\n", buffer);

		//printf("clear: %d\n", allClear);

	} while (allClear > 0);

	lex_rewind_pos = ftell(lex_file);

	// read a character and find out what token it is
	buffer[0] = '\0';
	fscanf(lex_file, "%1c", buffer);
	//printf("read character: %c\n", buffer[0]);
	buffer[1] = '\0';

	// start of command
	if (buffer[0] == '(') {
		//printf("command start: %s\n", buffer);
		return LEXER_TOKEN_COMMANDSTART;
	}
	else
	// end of command
	if (buffer[0] == ')') {
		//printf("command end: %s\n", buffer);
		return LEXER_TOKEN_COMMANDEND;
	}
	else
	// string
	if (buffer[0] == '\"') {
		fscanf(lex_file, "%499[^\"]", buffer);
		buffer[499] = '\0';
		fscanf(lex_file, "%*1c");
		//printf("found string: %s\n", buffer);
		return LEXER_TOKEN_STRING;
	}
	else
	// start of array
	if (buffer[0] == '[') {
		//printf("arrat start: %s\n", buffer);
		return LEXER_TOKEN_ARRAYSTART;
	}
	else
	// end of array
	if (buffer[0] == ']') {
		//printf("array end: %s\n", buffer);
		return LEXER_TOKEN_ARRAYEND;
	}
	else
	// period
	if (buffer[0] == '.') {
		//printf("period: %s\n", buffer);
		return LEXER_TOKEN_PERIOD;
	}
	else
	// identifier
	if ((buffer[0] >= 'a' && buffer[0] <= 'z')
	||  (buffer[0] >= 'A' && buffer[0] <= 'Z')
	||   buffer[0] == '_') {
		char restNumber[500];
		fscanf(lex_file, "%500[a-zA-Z0-9_]", restNumber);
		strcat(buffer, restNumber);
		//printf("identifier: %s\n", buffer);
		return LEXER_TOKEN_IDENTIFIER;
	}
	else
	// number
	if ((buffer[0] >= '0' && buffer[0] <= '9')) {

		// get the whole number
		char restNumber[500];
		restNumber[0] = '\0';
		if (fscanf(lex_file, "%499[0-9.]", restNumber)) {
			strcat(buffer, restNumber);
		}

		// decide if it's a floating number
		int isFloat = 0;
		char *ptr = buffer;
		while (ptr[0] != '\0') {
			if (ptr[0] == '.') {
				isFloat = 1;
				break;
			}
			ptr++;
		}

		// parsing float
		if (isFloat) {
			//printf("float: %s\n", buffer);
			return LEXER_TOKEN_FLOAT;
		}
		// parsing int
		else {
			//printf("int: %s\n", buffer);
			return LEXER_TOKEN_INT;
		}
	}
	else
	// special characters only meant for native commands
	if (buffer[0] == '-'
	||  buffer[0] == '+'
	||  buffer[0] == '/'
	||  buffer[0] == '*'
	||  buffer[0] == '%'
	||  buffer[0] == '='
	||  buffer[0] == '<'
	||  buffer[0] == '>'
	||  buffer[0] == '!'
	||  buffer[0] == '&'
	||  buffer[0] == '|') {

		// check if some symbols come with "="
		if (buffer[0] == '='
		||  buffer[0] == '<'
		||  buffer[0] == '>'
		||  buffer[0] == '!') {
			long pos = ftell(lex_file);
			char restId;
			fscanf(lex_file, "%1c", &restId);
			if (restId != '=') {
				fseek(lex_file, pos, SEEK_SET);
			}
			else {
				buffer[1] = restId;
			}
		}
		else
		if (buffer[0] == '&'
		||  buffer[0] == '|') {
			long pos = ftell(lex_file);
			char restId;
			fscanf(lex_file, "%1c", &restId);
			if (restId != buffer[0]) {
				fseek(lex_file, pos, SEEK_SET);
			}
			else {
				buffer[1] = restId;
			}
		}

		//printf("identifier special: %s\n", buffer);
		return LEXER_TOKEN_IDENTIFIER;
	}
	else
	// end of file -- nothing left to parse
	if (feof(lex_file)) {
		//printf("token done\n");
		return LEXER_TOKEN_DONE;
	}

	printf("unknown token: %s\n", buffer);
	return LEXER_TOKEN_UNKNOWN;
}

const char *lexer_getLexToken() {
	return buffer;
}

void lexer_rewind() {
	fseek(lex_file, lex_rewind_pos, SEEK_SET);
}
