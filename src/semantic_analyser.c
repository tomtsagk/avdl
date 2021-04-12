#include "semantic_analyser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

static struct ast_node *expect_identifier() {

	// confirm it's an identifier
	if (lexer_getNextToken() != LEXER_TOKEN_IDENTIFIER) {
		return 0;
	}

	// generate ast node for it
	struct ast_node *identifier = ast_create(AST_IDENTIFIER, 0);
	ast_addLex(identifier, lexer_getLexToken());

	// does it have an array modifier?

	// does it make an identifier chain? (myclass.myvar)
	if (lexer_getNextToken() != LEXER_TOKEN_PERIOD) {
		printf("identifier %s does not have a period\n", identifier->lex);
		lexer_rewind();
	}

	return identifier;

}

// looks for the structure of a command: (cmdname arg1 arg2 .. argN)
static struct ast_node *expect_command() {

	// confirm that a command is expected
	if (lexer_getNextToken() != LEXER_TOKEN_COMMANDSTART) {
		return 0;
	}

	// it's a command!
	struct ast_node *cmd = ast_create(0, 0);

	// the command's name has to be an identifier
	if (!expect_identifier(cmd)) {
		printf("syntax error, command name can only be an identifier\n");
		exit(-1);
	}

	while (lexer_getNextToken() != LEXER_TOKEN_COMMANDEND);

	//ast_child_add(node, cmd);

	printf("found command ~~~~~~~~~~~~~~~~~~~~~~\n");

	return cmd;
}

void semanticAnalyser_convertToAst(struct ast_node *node, const char *filename) {
	lexer_prepare(filename);

	struct ast_node *cmd;
	while (cmd = expect_command()) {
		ast_child_add(node, cmd);
	}
	/*
	int result;
	do {
		result = lexer_getNextToken();
		//printf("avdl error: known token: %s %d\n", lexer_getLexToken(), result);
	} while (result != LEXER_TOKEN_UNKNOWN && result != LEXER_TOKEN_DONE);

	if (result == LEXER_TOKEN_UNKNOWN) {
		printf("avdl error: unknown token: %s\n", lexer_getLexToken());
		exit(-1);
	}
	*/

	lexer_clean();
}
