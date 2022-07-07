#ifndef AVDL_LEXER_H
#define AVDL_LEXER_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Lexer
 * responsible for reading source code
 * and tokenizing it
 *
 * it also supports push new (included) files
 */

#define MAX_FILENAME_LENGTH 500

struct parsing_point {
	char tokenString[500];
	int tokenId;
	char filename[MAX_FILENAME_LENGTH];
	int lineNumber;
	int characterNumber;
};

struct file_properties {
	FILE *f;
	char filename[MAX_FILENAME_LENGTH];
	int currentLineNumber;
	int currentCharacterNumber;

	struct parsing_point pointLastRead;
};

/*
 * all available tokens the lexer understands
 */
enum lexer_tokens {
	LEXER_TOKEN_UNKNOWN,
	LEXER_TOKEN_DONE,

	LEXER_TOKEN_COMMANDSTART,
	LEXER_TOKEN_COMMANDEND,
	LEXER_TOKEN_ARRAYSTART,
	LEXER_TOKEN_ARRAYEND,
	LEXER_TOKEN_PERIOD,

	LEXER_TOKEN_STRING,
	LEXER_TOKEN_IDENTIFIER,
	LEXER_TOKEN_INT,
	LEXER_TOKEN_FLOAT,
};

struct avdl_lexer {
	int currentFile;
	struct file_properties files[10];
	struct parsing_point pointPrevious;
	struct parsing_point pointCurrent;

	char pastFiles[100][MAX_FILENAME_LENGTH];
	int currentPastFile;
};

/*
 * `create` initialises the lexer towards the given filename.
 * at some point `clean` should be called when the process is done.
 */
int avdl_lexer_create(struct avdl_lexer *, const char *filename);
void avdl_lexer_clean(struct avdl_lexer *);

/*
 * After initialisation, `getNextToken` will return the next
 * token.
 * `getLexToken` is to get the string that the token represents.
 *
 * peek is used to take a look at the next token, without
 * updating parsing position.
 */
int avdl_lexer_getNextToken(struct avdl_lexer *);
int avdl_lexer_peek(struct avdl_lexer *);
const char *avdl_lexer_getLexToken(struct avdl_lexer *);

const char *avdl_lexer_getCurrentFilename(struct avdl_lexer *);
int avdl_lexer_getCurrentLinenumber(struct avdl_lexer *);
int avdl_lexer_getCurrentCharacterNumber(struct avdl_lexer *);

void avdl_lexer_printCurrentLine(struct avdl_lexer *);

int avdl_lexer_addIncludedFile(struct avdl_lexer *o, const char *includeFilename);

#endif // lexer
