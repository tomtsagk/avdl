#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "dd_commands.h"
#include <ctype.h>

static char buffer[500];

static FILE *lex_file;
static char *currentFilename;
static int currentLineNumber;
static int currentCharacterNumber;

static long lex_pos_rewind;
static long lex_pos_previousLine;
static long lex_pos_currentLine;
static long lex_pos_current;

static int current_token;
static int rewind_token;

void lexer_prepare(const char *filename) {

	currentFilename = filename;

	buffer[0] = '\0';

	currentLineNumber = 1;
	currentCharacterNumber = 0;
	current_token = -1;
	rewind_token = -1;

	lex_file = 0;
	lex_file = fopen(filename, "r");
	if (!lex_file) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
		exit(-1);
	}

	lex_pos_rewind = lex_pos_previousLine = lex_pos_currentLine = lex_pos_current = ftell(lex_file);
}

void lexer_clean() {
	if (lex_file) {
		fclose(lex_file);
		lex_file = 0;
	}
}

int lexer_getNextToken() {

	if (rewind_token >= 0) {
		rewind_token = -1;
		return current_token;
	}

	currentCharacterNumber += strlen(buffer);
	//printf("current char number += %d, %s\n", strlen(buffer), buffer);

	// clear characters that are not tokens (whitespace / comments)
	int allClear;
	do {
		allClear = 0;

		// new line
		allClear += fscanf(lex_file, "%1[\n]", buffer);
		if (allClear) {
			//printf("ignored new line\n");
			currentLineNumber++;
			currentCharacterNumber = 0;
			lex_pos_previousLine = lex_pos_currentLine;
			lex_pos_currentLine = ftell(lex_file);
		}

		// ignore whitespace
		allClear += fscanf(lex_file, "%100[ \t]*", buffer);
		//printf("ignored whitespace: %s\n", buffer);

		// ignore comments
		allClear += fscanf(lex_file, "%1[#]%*[^\n]*", buffer);
		//printf("ignored comment: %s\n", buffer);

		//printf("clear: %d\n", allClear);

	} while (allClear > 0);

	lex_pos_rewind = ftell(lex_file);

	// read a character and find out what token it is
	buffer[0] = '\0';
	fscanf(lex_file, "%1c", buffer);
	//printf("read character: %c\n", buffer[0]);
	buffer[1] = '\0';

	int returnToken = LEXER_TOKEN_UNKNOWN;

	// start of command
	if (buffer[0] == '(') {
		//printf("command start: %s\n", buffer);
		//return LEXER_TOKEN_COMMANDSTART;
		returnToken =  LEXER_TOKEN_COMMANDSTART;
	}
	else
	// end of command
	if (buffer[0] == ')') {
		//printf("command end: %s\n", buffer);
		returnToken = LEXER_TOKEN_COMMANDEND;
	}
	else
	// string
	if (buffer[0] == '\"') {
		fscanf(lex_file, "%499[^\"]", buffer);
		buffer[499] = '\0';
		fscanf(lex_file, "%*1c");
		//printf("found string: %s\n", buffer);
		returnToken = LEXER_TOKEN_STRING;
	}
	else
	// start of array
	if (buffer[0] == '[') {
		//printf("arrat start: %s\n", buffer);
		returnToken = LEXER_TOKEN_ARRAYSTART;
	}
	else
	// end of array
	if (buffer[0] == ']') {
		//printf("array end: %s\n", buffer);
		returnToken = LEXER_TOKEN_ARRAYEND;
	}
	else
	// period
	if (buffer[0] == '.') {
		//printf("period: %s\n", buffer);
		returnToken = LEXER_TOKEN_PERIOD;
	}
	else
	// identifier
	if ((buffer[0] >= 'a' && buffer[0] <= 'z')
	||  (buffer[0] >= 'A' && buffer[0] <= 'Z')
	||   buffer[0] == '_') {
		char restNumber[500];
		if (fscanf(lex_file, "%500[a-zA-Z0-9_]", restNumber) > 0) {
			strcat(buffer, restNumber);
		}
		//printf("identifier: %s\n", buffer);
		returnToken = LEXER_TOKEN_IDENTIFIER;
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
			returnToken = LEXER_TOKEN_FLOAT;
		}
		// parsing int
		else {
			//printf("int: %s\n", buffer);
			returnToken = LEXER_TOKEN_INT;
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
		returnToken = LEXER_TOKEN_IDENTIFIER;
	}
	else
	// end of file -- nothing left to parse
	if (feof(lex_file)) {
		//printf("token done\n");
		returnToken = LEXER_TOKEN_DONE;
	}

	if (returnToken == LEXER_TOKEN_UNKNOWN) {
		printf("unknown token: %s\n", buffer);
		exit(-1);
	}

	lex_pos_current = ftell(lex_file);

	current_token = returnToken;
	return returnToken;
}

const char *lexer_getLexToken() {
	return buffer;
}

void lexer_rewind() {
	rewind_token = current_token;
	/*
	lex_pos_current	= lex_pos_rewind;
	fseek(lex_file, lex_pos_rewind, SEEK_SET);
	*/
}

const char *lexer_getCurrentFilename() {
	return currentFilename;
}

int lexer_getCurrentLinenumber() {
	return currentLineNumber;
}

/*
 * prints the previous and current source line,
 * with an arrow pointing on the first character of
 * the last parsed token
 */
void lexer_printCurrentLine() {

	// go to the line previous to the current one
	fseek(lex_file, lex_pos_previousLine, SEEK_SET);

	// print the previous and current line (on first line only print that)
	int extraLine = currentLineNumber > 1 ? 1 : 0;
	for (int i = 1 -extraLine; i < 2; i++) {
		char b[500];
		fscanf(lex_file, "%499[^\n]\n", b);
		b[499] = '\0';
		printf("   %d | %s\n", lexer_getCurrentLinenumber() -1 +i, b);
	}

	// print an arrow pointing at the current character
	char lineNum[20];
	sprintf(lineNum, "%d", lexer_getCurrentLinenumber());
	lineNum[19] = '\0';
	printf("      ");
	for (int i = 0; i < currentCharacterNumber +strlen(lineNum); i++) {
		printf(" ");
	}
	printf("^\n");

	// go back to (potentially) resume parsing
	fseek(lex_file, lex_pos_current, SEEK_SET);
}
