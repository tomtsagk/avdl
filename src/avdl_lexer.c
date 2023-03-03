#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "avdl_symtable.h"
#include "avdl_lexer.h"
#include "avdl_log.h"

static char buffer[500];

static int avdl_lexer_includePop(struct avdl_lexer *o) {

	if (o->currentFile == 0) {
		avdl_log_error("lexer: attempted to pop base file");
		return -1;
	}

	if (o->files[o->currentFile].f) {
		fclose(o->files[o->currentFile].f);
		o->files[o->currentFile].f = 0;
	}

	o->currentFile--;

	// restore cache
	strcpy(o->pointCurrent.tokenString, o->files[o->currentFile].pointLastRead.tokenString);
	o->pointCurrent.tokenId = o->files[o->currentFile].pointLastRead.tokenId;
	strcpy(o->pointCurrent.filename, o->files[o->currentFile].pointLastRead.filename);
	o->pointCurrent.lineNumber = o->files[o->currentFile].pointLastRead.lineNumber;
	o->pointCurrent.characterNumber = o->files[o->currentFile].pointLastRead.characterNumber;

	return 0;
}

int avdl_lexer_create(struct avdl_lexer *o, const char *filename) {

	if (strlen(filename) > MAX_FILENAME_LENGTH-1) {
		avdl_log_error("lexer: cannot initiate, filename too big: '%s'", filename);
		return -1;
	}
	o->currentFile = 0;
	strcpy(o->files[0].filename, filename);
	o->files[0].currentLineNumber = 1;
	o->files[0].currentCharacterNumber = 1;

	o->files[0].f = fopen(filename, "r");
	if (!o->files[0].f) {
		avdl_log_error("lexer: unable to open '%s': %s", filename, strerror(errno));
		return -1;
	}

	o->pointPrevious.tokenString[0] = '\0';
	o->pointPrevious.tokenId = -1;
	o->pointPrevious.filename[0] = '\0';
	o->pointPrevious.lineNumber = 0;
	o->pointPrevious.characterNumber = 1;

	o->pointCurrent.tokenString[0] = '\0';
	o->pointCurrent.tokenId = -1;
	o->pointCurrent.filename[0] = '\0';
	o->pointCurrent.lineNumber = 0;
	o->pointCurrent.characterNumber = 1;

	o->currentPastFile = -1;

	avdl_lexer_getNextToken(o);
	return 0;
}

int avdl_lexer_getNextToken(struct avdl_lexer *o) {

	// clear characters that are not tokens (whitespace / comments)
	int allClear;
	do {
		allClear = 0;

		// new line
		allClear += fscanf(o->files[o->currentFile].f, "%1[\n]", buffer);
		if (allClear) {
			//printf("ignored new line: %d %d\n", currentFile, files[currentFile].currentLineNumber);
			o->files[o->currentFile].currentLineNumber++;
			o->files[o->currentFile].currentCharacterNumber = 1;
		}

		// ignore whitespace
		buffer[0] = '\0';
		allClear += fscanf(o->files[o->currentFile].f, "%100[ \t]*", buffer);
		o->files[o->currentFile].currentCharacterNumber += strlen(buffer);
		//printf("ignored whitespace: %s\n", buffer);

		// ignore comments
		buffer[0] = '\0';
		allClear += fscanf(o->files[o->currentFile].f, "%1[#]%*[^\n]*", buffer);
		o->files[o->currentFile].currentCharacterNumber += strlen(buffer);
		//printf("ignored comment: %s\n", buffer);

		//printf("clear: %d\n", allClear);

		// reached EOF in included file
		if (feof(o->files[o->currentFile].f) && o->currentFile > 0) {
			strcpy(o->pointPrevious.tokenString, o->pointCurrent.tokenString);
			o->pointPrevious.tokenId = o->pointCurrent.tokenId;
			strcpy(o->pointPrevious.filename, o->pointCurrent.filename);
			o->pointPrevious.lineNumber = o->pointCurrent.lineNumber;
			o->pointPrevious.characterNumber = o->pointCurrent.characterNumber;
			if (avdl_lexer_includePop(o) != 0) {
				return -1;
			}
			return o->pointPrevious.tokenId;
		}

	} while (allClear > 0);

	// read a character and find out what token it is
	buffer[0] = '\0';
	fscanf(o->files[o->currentFile].f, "%1c", buffer);
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
		fscanf(o->files[o->currentFile].f, "%499[^\"]", buffer);
		buffer[499] = '\0';
		fscanf(o->files[o->currentFile].f, "%*1c");
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
		if (fscanf(o->files[o->currentFile].f, "%500[a-zA-Z0-9_]", restNumber) > 0) {
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
		if (fscanf(o->files[o->currentFile].f, "%499[0-9.]", restNumber) > 0) {
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
			char restNumber[500];
			if (fscanf(o->files[o->currentFile].f, "%499[0-9.]", restNumber) > 0) {
				buffer[1] = '\0';
				strcat(buffer, restNumber);

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
					//printf("neg float: %s\n", buffer);
					returnToken = LEXER_TOKEN_FLOAT;
				}
				// parsing int
				else {
					//printf("neg int: %s\n", buffer);
					returnToken = LEXER_TOKEN_INT;
				}
			}
		}
		else
		// check if some symbols come with "="
		if (buffer[0] == '='
		||  buffer[0] == '<'
		||  buffer[0] == '>'
		||  buffer[0] == '!') {
			char restId;
			if (fscanf(o->files[o->currentFile].f, "%1[=]", &restId)) {
				buffer[1] = restId;
				buffer[2] = '\0';
			}
		}
		else
		if (buffer[0] == '&'
		||  buffer[0] == '|') {
			char restId;
			if (buffer[0] == '&') {
				if (fscanf(o->files[o->currentFile].f, "%1[&]", &restId)) {
					buffer[1] = restId;
					buffer[2] = '\0';
				}
			}
			else
			if (buffer[0] == '|') {
				if (fscanf(o->files[o->currentFile].f, "%1[|]", &restId)) {
					buffer[1] = restId;
					buffer[2] = '\0';
				}
			}
		}

		//printf("identifier special: %s\n", buffer);
		if (returnToken == LEXER_TOKEN_UNKNOWN) {
			returnToken = LEXER_TOKEN_IDENTIFIER;
		}
	}
	else
	// end of file -- nothing left to parse
	if (feof(o->files[o->currentFile].f)) {
		//printf("token done\n");
		returnToken = LEXER_TOKEN_DONE;
	}

	if (returnToken == LEXER_TOKEN_UNKNOWN) {
		avdl_log_error("lexer: unknown token: %s", buffer);
		return -1;
	}

	// returnToken - token id
	// buffer - token string

	if (o->pointCurrent.tokenId != -1) {
		strcpy(o->pointPrevious.tokenString, o->pointCurrent.tokenString);
		o->pointPrevious.tokenId = o->pointCurrent.tokenId;
		strcpy(o->pointPrevious.filename, o->pointCurrent.filename);
		o->pointPrevious.lineNumber = o->pointCurrent.lineNumber;
		o->pointPrevious.characterNumber = o->pointCurrent.characterNumber;
	}
	o->pointCurrent.tokenId = returnToken;
	strcpy(o->pointCurrent.tokenString, buffer);
	strcpy(o->pointCurrent.filename, o->files[o->currentFile].filename);
	o->pointCurrent.lineNumber = o->files[o->currentFile].currentLineNumber;
	o->pointCurrent.characterNumber = o->files[o->currentFile].currentCharacterNumber;

	//printf("lexer %d: %s %d\n", o->currentFile, o->pointCurrent.tokenString, o->pointCurrent.tokenId);
	//lexer_printCurrentLine();

	o->files[o->currentFile].currentCharacterNumber += strlen(buffer);
	if (returnToken == LEXER_TOKEN_STRING) {
		o->files[o->currentFile].currentCharacterNumber += 2;
	}
	//printf("current char number += %d, %s = %d\n", strlen(buffer), buffer, files[currentFile].currentCharacterNumber);

	return o->pointPrevious.tokenId;
}

int avdl_lexer_peek(struct avdl_lexer *o) {
	return o->pointCurrent.tokenId;
}

const char *avdl_lexer_getLexToken(struct avdl_lexer *o) {
	return o->pointPrevious.tokenString;
}

const char *avdl_lexer_getCurrentFilename(struct avdl_lexer *o) {
	return o->pointPrevious.filename;
}

int avdl_lexer_getCurrentLinenumber(struct avdl_lexer *o) {
	return o->pointPrevious.lineNumber;
}

int avdl_lexer_getCurrentCharacterNumber(struct avdl_lexer *o) {
	return o->pointPrevious.characterNumber;
}

/*
 * prints the previous and current source line,
 * with an arrow pointing on the first character of
 * the last parsed token
 */
void avdl_lexer_printCurrentLine(struct avdl_lexer *o) {

	FILE *f = fopen(o->pointPrevious.filename, "r");
	char b[500];
	if (!f) {
		avdl_log_error("lexer: error opening file for logging errors '%s': %s", o->pointPrevious.filename, strerror(errno));
		return;
	}

	for (int i = 1; i < o->pointPrevious.lineNumber-1; i++) {
		fgets(b, 499, f);
	}

	//printf("avdl: error during compilation at %s:%d:%d\n", pointPrevious.filename, pointPrevious.lineNumber, pointPrevious.characterNumber);
	for (int i = o->pointPrevious.lineNumber == 1 ? 1 : 0; i < 3; i++) {
		fgets(b, 499, f);
		char *p = b;
		while (p[0] != '\0') {
			if (p[0] == '\n' || p[0] == '\r') {
				p[0] = '\0';
			}
			else
			if (p[0] == '\t') {
				p[0] = ' ';
			}
			p++;
		}
		avdl_log("|%s", b);
		if (i == 1) {
			printf(" ");
			for (int j = 0; j < o->pointPrevious.characterNumber-1; j++) {
				printf(" ");
			}
			printf("^\n");
		}
	}

	//printf("error: { %d %s } in %s:%d:%d\n", pointPrevious.tokenId, pointPrevious.tokenString, pointPrevious.filename, pointPrevious.lineNumber, pointPrevious.characterNumber);
}

int avdl_lexer_addIncludedFile(struct avdl_lexer *o, const char *includeFilename) {

	// skip files that have been included before
	for (int i = 0; i <= o->currentPastFile; i++) {
		if (strcmp(o->pastFiles[i], includeFilename) == 0) {
			return 0;
		}
	}

	// character limit per filename
	if (strlen(includeFilename) >= MAX_FILENAME_LENGTH-1) {
		avdl_log_error("lexer: cannot include %s: too long name", includeFilename);
		return -1;
	}

	// save included file so it's not included again
	o->currentPastFile++;
	strcpy(o->pastFiles[o->currentPastFile], includeFilename);

	// temp commented out
	strcpy(buffer, "include/");
	strcat(buffer, includeFilename);
	/*
	if (includePath) {
		strcpy(buffer, includePath);
		strcat(buffer, includeFilename);
	}
	else {
		strcpy(buffer, includeFilename);
	}
	*/

	if (o->currentFile+1 >= 10) {
		avdl_log_error("lexer: reached limit of included files with: '%s'", buffer);
		return -1;
	}

	// cache previous token
	strcpy(o->files[o->currentFile].pointLastRead.tokenString, o->pointCurrent.tokenString);
	o->files[o->currentFile].pointLastRead.tokenId = o->pointCurrent.tokenId;
	strcpy(o->files[o->currentFile].pointLastRead.filename, o->pointCurrent.filename);
	o->files[o->currentFile].pointLastRead.lineNumber = o->pointCurrent.lineNumber;
	o->files[o->currentFile].pointLastRead.characterNumber = o->pointCurrent.characterNumber;

	o->currentFile++;
	strcpy(o->files[o->currentFile].filename, buffer);
	o->files[o->currentFile].currentLineNumber = 1;
	o->files[o->currentFile].currentCharacterNumber = 0;

	o->files[o->currentFile].f = fopen(buffer, "r");
	if (!o->files[o->currentFile].f) {
		avdl_log_error("lexer: Unable to open '%s': %s", buffer, strerror(errno));
		return -1;
	}

	avdl_lexer_getNextToken(o);
	return 0;
}

void avdl_lexer_clean(struct avdl_lexer *o) {
	for (int i = 0; i <= o->currentFile; i++) {
		if (o->files[i].f) {
			fclose(o->files[i].f);
			o->files[i].f = 0;
		}
	}
	o->currentFile = -1;
}
