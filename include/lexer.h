#ifndef AVDL_LEXER_H
#define AVDL_LEXER_H

#include "ast_node.h"

/*
 * Lexer
 * Responsible for reading source code
 * and tokenizing it, so it's more straightforward
 * to do semantic analysis on it
 */

// all available tokens `avdl` understands
enum lexer_tokens {
	LEXER_TOKEN_DONE,
	LEXER_TOKEN_UNKNOWN,
	LEXER_TOKEN_COMMANDSTART,
	LEXER_TOKEN_COMMANDEND,
	LEXER_TOKEN_STRING,
	LEXER_TOKEN_ARRAYSTART,
	LEXER_TOKEN_ARRAYEND,
	LEXER_TOKEN_PERIOD,
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
 */
int lexer_getNextToken();
void lexer_rewind();
const char *lexer_getLexToken();

#endif // lexer
