#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "dd_commands.h"
#include <ctype.h>

static char buffer[500];

struct file_properties {
	FILE *f;
	const char *filename;
	int currentLineNumber;
	int currentCharacterNumber;

	long lex_pos_rewind;
	long lex_pos_previousLine;
	long lex_pos_currentLine;
	long lex_pos_current;

	int current_token;
	int rewind_token;
};

static struct file_properties files[10];
static int currentFile = -1;

static void lexer_includePop() {

	if (currentFile == 0) {
		printf("lexer: attempted to pop base file\n");
		exit(-1);
	}

	if (files[currentFile].f) {
		fclose(files[currentFile].f);
		files[currentFile].f = 0;
	}

	currentFile--;
}

void lexer_prepare(const char *filename) {
	currentFile = 0;
	files[0].filename = filename;
	files[0].currentLineNumber = 1;
	files[0].currentCharacterNumber = 0;
	files[0].current_token = -1;
	files[0].rewind_token = -1;

	files[0].f = fopen(filename, "r");
	if (!files[0].f) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
		exit(-1);
	}

	files[0].lex_pos_rewind =
		files[0].lex_pos_previousLine =
			files[0].lex_pos_currentLine =
				files[0].lex_pos_current =
					ftell(files[0].f);
}

void lexer_clean() {
	for (int i = 0; i < currentFile; i++) {
		if (files[i].f) {
			fclose(files[i].f);
			files[i].f = 0;
		}
	}
	currentFile = -1;
}

int lexer_getNextToken() {

	if (files[currentFile].rewind_token >= 0) {
		files[currentFile].rewind_token = -1;
		return files[currentFile].current_token;
	}

	files[currentFile].currentCharacterNumber += strlen(buffer);
	//printf("current char number += %d, %s\n", strlen(buffer), buffer);

	// clear characters that are not tokens (whitespace / comments)
	int allClear;
	do {
		allClear = 0;

		// new line
		allClear += fscanf(files[currentFile].f, "%1[\n]", buffer);
		if (allClear) {
			//printf("ignored new line\n");
			files[currentFile].currentLineNumber++;
			files[currentFile].currentCharacterNumber = 0;
			files[currentFile].lex_pos_previousLine = files[currentFile].lex_pos_currentLine;
			files[currentFile].lex_pos_currentLine = ftell(files[currentFile].f);
		}

		// ignore whitespace
		allClear += fscanf(files[currentFile].f, "%100[ \t]*", buffer);
		//printf("ignored whitespace: %s\n", buffer);

		// ignore comments
		allClear += fscanf(files[currentFile].f, "%1[#]%*[^\n]*", buffer);
		//printf("ignored comment: %s\n", buffer);

		//printf("clear: %d\n", allClear);

		// reached EOF in included file
		if (feof(files[currentFile].f) && currentFile > 0) {
			lexer_includePop();
			return lexer_getNextToken();
		}

	} while (allClear > 0);

	files[currentFile].lex_pos_rewind = ftell(files[currentFile].f);

	// read a character and find out what token it is
	buffer[0] = '\0';
	fscanf(files[currentFile].f, "%1c", buffer);
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
		fscanf(files[currentFile].f, "%499[^\"]", buffer);
		buffer[499] = '\0';
		fscanf(files[currentFile].f, "%*1c");
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
		if (fscanf(files[currentFile].f, "%500[a-zA-Z0-9_]", restNumber) > 0) {
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
		if (fscanf(files[currentFile].f, "%499[0-9.]", restNumber) > 0) {
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
			long pos = ftell(files[currentFile].f);
			char restId;
			fscanf(files[currentFile].f, "%1c", &restId);
			if (restId != '=') {
				fseek(files[currentFile].f, pos, SEEK_SET);
			}
			else {
				buffer[1] = restId;
			}
		}
		else
		if (buffer[0] == '&'
		||  buffer[0] == '|') {
			long pos = ftell(files[currentFile].f);
			char restId;
			fscanf(files[currentFile].f, "%1c", &restId);
			if (restId != buffer[0]) {
				fseek(files[currentFile].f, pos, SEEK_SET);
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
	if (feof(files[currentFile].f)) {
		//printf("token done\n");
		returnToken = LEXER_TOKEN_DONE;
	}

	if (returnToken == LEXER_TOKEN_UNKNOWN) {
		printf("unknown token: %s\n", buffer);
		exit(-1);
	}

	files[currentFile].lex_pos_current = ftell(files[currentFile].f);

	files[currentFile].current_token = returnToken;
	return returnToken;
}

const char *lexer_getLexToken() {
	return buffer;
}

void lexer_rewind() {
	files[currentFile].rewind_token = files[currentFile].current_token;
	//rewind_token = current_token;
	/*
	lex_pos_current	= lex_pos_rewind;
	fseek(lex_file, lex_pos_rewind, SEEK_SET);
	*/
}

const char *lexer_getCurrentFilename() {
	return files[currentFile].filename;
	//return currentFilename;
}

int lexer_getCurrentLinenumber() {
	return files[currentFile].currentLineNumber;
	//return currentLineNumber;
}

/*
 * prints the previous and current source line,
 * with an arrow pointing on the first character of
 * the last parsed token
 */
void lexer_printCurrentLine() {

	// go to the line previous to the current one
	fseek(files[currentFile].f, files[currentFile].lex_pos_previousLine, SEEK_SET);

	// print the previous and current line (on first line only print that)
	int extraLine = files[currentFile].currentLineNumber > 1 ? 1 : 0;
	for (int i = 1 -extraLine; i < 2; i++) {
		char b[500];
		fscanf(files[currentFile].f, "%499[^\n]\n", b);
		b[499] = '\0';
		printf("   %d | %s\n", lexer_getCurrentLinenumber() -1 +i, b);
	}

	// print an arrow pointing at the current character
	char lineNum[20];
	sprintf(lineNum, "%d", lexer_getCurrentLinenumber());
	lineNum[19] = '\0';
	printf("      ");
	for (int i = 0; i < files[currentFile].currentCharacterNumber +strlen(lineNum); i++) {
		printf(" ");
	}
	printf("^\n");

	// go back to (potentially) resume parsing
	fseek(files[currentFile].f, files[currentFile].lex_pos_current, SEEK_SET);
}

void lexer_addIncludedFile(const char *includeFilename) {

	if (currentFile+1 >= 10) {
		printf("lexer: reached limit of included files with: '%s'\n", includeFilename);
		exit(-1);
	}

	currentFile++;
	files[currentFile].filename = includeFilename;
	files[currentFile].currentLineNumber = 1;
	files[currentFile].currentCharacterNumber = 0;
	files[currentFile].current_token = -1;
	files[currentFile].rewind_token = -1;

	files[currentFile].f = fopen(includeFilename, "r");
	if (!files[currentFile].f) {
		printf("avdl error: Unable to open '%s': %s\n", includeFilename, strerror(errno));
		exit(-1);
	}

	files[currentFile].lex_pos_rewind =
		files[currentFile].lex_pos_previousLine =
			files[currentFile].lex_pos_currentLine =
				files[currentFile].lex_pos_current =
					ftell(files[currentFile].f);
}
