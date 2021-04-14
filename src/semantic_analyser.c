#include "semantic_analyser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "struct_table.h"
#include "dd_commands.h"
#include <stdarg.h>

static struct ast_node *expect_command_definition();
static struct ast_node *expect_identifier();
static struct ast_node *expect_command();
static void semantic_error(const char *msg, ...);

static struct ast_node *expect_command_definition() {

	struct ast_node *definition = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(definition, "def");

	// get type
	struct ast_node *type = expect_identifier();

	// check if primitive or struct
	if (!dd_variable_type_isPrimitiveType(type->lex)) {
		semantic_error("unrecognized type '%s'", type->lex);
	}

	// get variable name
	struct ast_node *varname = expect_identifier();

	// add newly defined variable to symbol table
	struct entry *e = symtable_entryat(symtable_insert(varname->lex, SYMTABLE_VARIABLE));
	e->value = dd_variable_type_convert(type->lex);

	ast_child_add(definition, type);
	ast_child_add(definition, varname);
	return definition;
}

static struct ast_node *expect_identifier() {

	// confirm it's an identifier
	if (lexer_getNextToken() != LEXER_TOKEN_IDENTIFIER) {
		semantic_error("expected identifier instead of '%s'", lexer_getLexToken());
	}

	// generate ast node for it
	struct ast_node *identifier = ast_create(AST_IDENTIFIER, 0);
	ast_addLex(identifier, lexer_getLexToken());

	// does it have an array modifier?
	if (lexer_getNextToken() == LEXER_TOKEN_ARRAYSTART) {
		struct ast_node *array = ast_create(AST_GROUP, 0);
		while (lexer_getNextToken() != LEXER_TOKEN_ARRAYEND);
		ast_child_add(identifier, array);
	}
	else {
		lexer_rewind();
	}

	// it has a period (myclass.myvar)
	if (lexer_getNextToken() == LEXER_TOKEN_PERIOD) {
		struct ast_node *child = expect_identifier();
		ast_child_add(identifier, child);
	}
	// it does not have a period
	else {
		lexer_rewind();
	}

	return identifier;
}

// looks for the structure of a command: (cmdname arg1 arg2 .. argN)
static struct ast_node *expect_command() {

	// confirm that a command is expected
	if (lexer_getNextToken() != LEXER_TOKEN_COMMANDSTART) {
		semantic_error("expected the start of a command '('");
	}

	// get the command's name
	struct ast_node *cmd;
	struct ast_node *cmdname = expect_identifier();

	// native command, special rules
	if (dd_commands_isNative(cmdname->lex)) {

		// native command can only be a name, no array modifiers or owning data
		if (cmdname->children.elements > 0) {
			semantic_error("native command name cannot have an array modifier, or own other data\n");
		}

		// based on the command, expect different data
		if (strcmp(cmdname->lex, "def") == 0) {
			cmd = expect_command_definition();
		}
		else {
			semantic_error("no rule to parse command '%s'", cmdname->lex);
		}
	}
	// custom command, make sure it exists
	else {
		if (symtable_lookup(cmdname->lex) == -1) {
			printf("unrecognized identifier: %s\n", cmdname->lex);
			exit(-1);
		}
		cmd = ast_create(0, 0);
		cmd->node_type = AST_COMMAND_CUSTOM;
		ast_child_add(cmd, cmdname);
	}

	// get the command's children
	struct ast_node *child = 0;
	int token = 0;
	if ((token = lexer_getNextToken()) != LEXER_TOKEN_COMMANDEND) {
		printf("was expected command end\n");
		exit(-1);
	}

	return cmd;
}

void semanticAnalyser_convertToAst(struct ast_node *node, const char *filename) {

	struct_table_init();
	symtable_init();
	lexer_prepare(filename);

	struct ast_node *cmd;
	while (lexer_getNextToken() == LEXER_TOKEN_COMMANDSTART) {
		lexer_rewind();
		cmd = expect_command();
		ast_child_add(node, cmd);
	}

	symtable_print();
	struct_table_print();
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

static void semantic_error(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	printf("avdl syntax error:\n");
	printf("%s:%d: ", lexer_getCurrentFilename(), lexer_getCurrentLinenumber());
	vprintf(msg, args);
	printf("\n");
	lexer_printCurrentLine();
	exit(-1);
}
