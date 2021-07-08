#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include <ctype.h>

extern char *includePath;

static char buffer[500];
static char lastTokenRead[200];
static int lastToken;

struct file_properties {
	FILE *f;
	char filename[200];
	int currentLineNumber;
	int currentCharacterNumber;

	long lex_pos_previousLine;
	long lex_pos_currentLine;
	long lex_pos_current;
};

static char pastFiles[100][100];
static int currentPastFile = -1;
static struct file_properties files[10];
static int currentFile = -1;

static void lexer_includePop(int peek) {

	if (currentFile == 0) {
		printf("lexer: attempted to pop base file\n");
		exit(-1);
	}

	if (files[currentFile].f && !peek) {
		fclose(files[currentFile].f);
		files[currentFile].f = 0;
	}

	currentFile--;
}

void lexer_prepare(const char *filename) {
	lastTokenRead[0] = '\0';
	lastToken = LEXER_TOKEN_UNKNOWN;
	currentFile = 0;
	strcpy(files[0].filename, filename);
	files[0].currentLineNumber = 1;
	files[0].currentCharacterNumber = 0;

	files[0].f = fopen(filename, "r");
	if (!files[0].f) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
		exit(-1);
	}

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

static int getNextToken(int peek) {

	files[currentFile].currentCharacterNumber += strlen(lastTokenRead);
	if (lastToken == LEXER_TOKEN_STRING) {
		files[currentFile].currentCharacterNumber += 2;
	}
	//printf("current char number += %d, %s = %d\n", strlen(buffer), buffer, files[currentFile].currentCharacterNumber);

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
		buffer[0] = '\0';
		allClear += fscanf(files[currentFile].f, "%100[ \t]*", buffer);
		files[currentFile].currentCharacterNumber += strlen(buffer);
		//printf("ignored whitespace: %s\n", buffer);

		// ignore comments
		buffer[0] = '\0';
		allClear += fscanf(files[currentFile].f, "%1[#]%*[^\n]*", buffer);
		files[currentFile].currentCharacterNumber += strlen(buffer);
		//printf("ignored comment: %s\n", buffer);

		//printf("clear: %d\n", allClear);

		// reached EOF in included file
		if (feof(files[currentFile].f) && currentFile > 0) {
			lexer_includePop(peek);
			return lexer_getNextToken();
		}

	} while (allClear > 0);

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

		// check if negative number
		if (buffer[0] == '-') {
			long pos = ftell(files[currentFile].f);
			char restId;
			fscanf(files[currentFile].f, "%1c", &restId);
			if (restId >= '0' && restId <= '9') {
				buffer[1] = restId;
				buffer[2] = '\0';
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
			else {
				fseek(files[currentFile].f, pos, SEEK_SET);
			}
		}
		else
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
				buffer[2] = '\0';
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
				buffer[2] = '\0';
			}
		}

		//printf("identifier special: %s\n", buffer);
		if (returnToken == LEXER_TOKEN_UNKNOWN) {
			returnToken = LEXER_TOKEN_IDENTIFIER;
		}
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

	if (!peek) {
		strncpy(lastTokenRead, buffer, 199);
		lastTokenRead[199] = '\0';
		lastToken = returnToken;
	}

	return returnToken;
}

int lexer_getNextToken() {
	return getNextToken(0);
}

int lexer_peek() {

	struct file_properties files_backup[10];
	memcpy(files_backup, files, sizeof(files_backup));
	int currentFile_backup = currentFile;

	int token = getNextToken(1);

	memcpy(files, files_backup, sizeof(files_backup));
	currentFile = currentFile_backup;

	for (int i = 0; i <= currentFile; i++) {
		fseek(files[i].f, files[i].lex_pos_current, SEEK_SET);
	}

	return token;
}

const char *lexer_getLexToken() {
	return lastTokenRead;
}

const char *lexer_getCurrentFilename() {
	return files[currentFile].filename;
}

int lexer_getCurrentLinenumber() {
	return files[currentFile].currentLineNumber;
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

	// skip files that have been included before
	for (int i = 0; i <= currentPastFile; i++) {
		if (strcmp(pastFiles[i], includeFilename) == 0) {
			return;
		}
	}

	// temp limit of 100 characters per filename
	if (strlen(includeFilename) >= 100) {
		printf("cannot include %s: too long name\n", includeFilename);
		exit(-1);
	}

	// save included file so it's not included again
	currentPastFile++;
	strcpy(pastFiles[currentFile], includeFilename);

	if (includePath) {
		strcpy(buffer, includePath);
		strcat(buffer, includeFilename);
	}
	else {
		strcpy(buffer, includeFilename);
	}

	if (currentFile+1 >= 10) {
		printf("lexer: reached limit of included files with: '%s'\n", buffer);
		exit(-1);
	}

	currentFile++;
	strcpy(files[currentFile].filename, buffer);
	files[currentFile].currentLineNumber = 1;
	files[currentFile].currentCharacterNumber = 0;

	files[currentFile].f = fopen(buffer, "r");
	if (!files[currentFile].f) {
		printf("avdl error: Unable to open '%s': %s\n", buffer, strerror(errno));
		exit(-1);
	}

	files[currentFile].lex_pos_previousLine =
		files[currentFile].lex_pos_currentLine =
			files[currentFile].lex_pos_current =
				ftell(files[currentFile].f);
}
