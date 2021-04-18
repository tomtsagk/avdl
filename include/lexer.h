#ifndef AVDL_LEXER_H
#define AVDL_LEXER_H

/*
 * Lexer
 * responsible for reading source code
 * and tokenizing it
 *
 * it also supports push new (included) files
 */

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

/*
 * `prepare` initialises the lexer towards the given filename.
 * at some point `clean` should be called when the process is done.
 */
void lexer_prepare(const char *filename);
void lexer_clean();

/*
 * After initialisation, `getNextToken` will return the next
 * token.
 * In case it's not needed, `rewind` will go back one step, so
 * `getNextToken` will return the same token when called again.
 * `getLexToken` is to get the string that the token represents.
 *
 * peek is used to take a look at the next token, without
 * updating parsing position.
 */
int lexer_getNextToken();
int lexer_peek();
const char *lexer_getLexToken();

const char *lexer_getCurrentFilename();
int lexer_getCurrentLinenumber();

void lexer_printCurrentLine();

void lexer_addIncludedFile(const char *includeFilename);

#endif // lexer
