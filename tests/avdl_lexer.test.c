#include "avdl_symtable.h"
#include "avdl_lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void expect_token(struct avdl_lexer *l, int token, const char *tokenstr, const char *filename, int lineNumber, int charNumber) {
	assert(avdl_lexer_peek(l) == token);
	assert(avdl_lexer_getNextToken(l) == token);
	assert(strcmp(avdl_lexer_getLexToken(l), tokenstr) == 0);
	assert(strcmp(avdl_lexer_getCurrentFilename(l), filename) == 0);
	assert(avdl_lexer_getCurrentLinenumber(l) == lineNumber);
	assert(avdl_lexer_getCurrentCharacterNumber(l) == charNumber);
}

int main(int argc, char *argv[]) {

	struct avdl_lexer l;

	// test a sample file
	const char *filename = "tests/avdl_lexer_input.dd";
	assert(avdl_lexer_create(&l, filename) == 0);

	int lineCount = 0;
	int characterCount = 0;

	lineCount = 1;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_COMMANDSTART, "(", filename, lineCount, characterCount);

	lineCount = 1;
	characterCount = 2;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "def", filename, lineCount, characterCount);

	lineCount = 1;
	characterCount = 6;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "int", filename, lineCount, characterCount);

	lineCount = 1;
	characterCount = 10;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "x", filename, lineCount, characterCount);

	lineCount = 1;
	characterCount = 11;
	expect_token(&l, LEXER_TOKEN_COMMANDEND, ")", filename, lineCount, characterCount);

	lineCount = 2;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_STRING, "test string", filename, lineCount, characterCount);

	lineCount = 3;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_ARRAYSTART, "[", filename, lineCount, characterCount);

	lineCount = 4;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_ARRAYEND, "]", filename, lineCount, characterCount);

	lineCount = 5;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_PERIOD, ".", filename, lineCount, characterCount);

	lineCount = 6;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_INT, "12345", filename, lineCount, characterCount);

	lineCount = 7;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_FLOAT, "123.45", filename, lineCount, characterCount);

	lineCount = 8;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_INT, "-16", filename, lineCount, characterCount);

	lineCount = 9;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_FLOAT, "-11.43", filename, lineCount, characterCount);

	lineCount = 10;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "==", filename, lineCount, characterCount);

	lineCount = 11;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "&&", filename, lineCount, characterCount);

	lineCount = 12;
	characterCount = 1;
	expect_token(&l, LEXER_TOKEN_IDENTIFIER, "||", filename, lineCount, characterCount);

	avdl_lexer_clean(&l);

	// test error values
	char hugefilename[MAX_FILENAME_LENGTH+1];
	memset(hugefilename, 'a', MAX_FILENAME_LENGTH);
	hugefilename[MAX_FILENAME_LENGTH] = '\0';
	assert(avdl_lexer_create(&l, hugefilename) != 0);

	return 0;
}
