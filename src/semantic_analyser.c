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
static struct ast_node *expect_command_classDefinition();
static struct ast_node *expect_command_group();
static struct ast_node *expect_command_functionDefinition();
static struct ast_node *expect_command_classFunction();
static struct ast_node *expect_identifier();
static struct ast_node *expect_int();
static struct ast_node *expect_float();
static struct ast_node *expect_string();
static struct ast_node *expect_command_binaryOperation();
static struct ast_node *expect_command();
static struct ast_node *expect_command_arg();
static struct ast_node *expect_command_if();
static struct ast_node *expect_command_include();
static void semantic_error(const char *msg, ...);

static struct ast_node *expect_command_include() {
	struct ast_node *include = ast_create(AST_INCLUDE, 0);
	struct ast_node *filename = expect_string();
	ast_delete(filename);
	ast_addLex(include, filename->lex);
	return include;
}

static struct ast_node *expect_command_if() {

	struct ast_node *ifcmd = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(ifcmd, "if");

	while (lexer_peek() != LEXER_TOKEN_COMMANDEND) {
		ast_child_add(ifcmd, expect_command_arg());
	}

	return ifcmd;
}

static struct ast_node *expect_command_classFunction() {

	struct ast_node *classFunc = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(classFunc, "class_function");

	struct ast_node *classname = expect_identifier();
	struct ast_node *function = expect_command_functionDefinition();

	symtable_push();
	struct entry *e = symtable_entryat(symtable_insert("this", DD_VARIABLE_TYPE_STRUCT));
	e->isRef = 1;
	struct ast_node *functionStatements = expect_command();
	symtable_pop();

	ast_child_add(classFunc, classname);
	ast_child_add(function, functionStatements);
	ast_child_add(classFunc, function);

	return classFunc;
}

static struct ast_node *expect_command_functionDefinition() {

	struct ast_node *function = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(function, "function");

	struct ast_node *returnType = expect_identifier();
	struct ast_node *functionName = expect_identifier();
	struct ast_node *args = expect_command();

	ast_child_add(function, returnType);
	ast_child_add(function, functionName);
	ast_child_add(function, args);

	return function;
}

static struct ast_node *expect_command_group() {

	struct ast_node *group = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(group, "group");

	while (lexer_peek() != LEXER_TOKEN_COMMANDEND) {
		ast_child_add(group, expect_command_arg());
	}

	return group;
}

static struct ast_node *expect_command_classDefinition() {

	struct ast_node *classname = expect_identifier();
	struct ast_node *subclassname = expect_identifier();

	symtable_push();
	struct ast_node *definitions = expect_command();
	symtable_pop();

	int structIndex = struct_table_push(classname->lex, subclassname->lex);

	for (int i = 0; i < definitions->children.elements; i++) {
		struct ast_node *child = dd_da_get(&definitions->children, i);
		struct ast_node *type = dd_da_get(&child->children, 0);
		struct ast_node *name = dd_da_get(&child->children, 1);

		if (strcmp(child->lex, "def") == 0) {
			//printf("variable: %s %s\n", type->lex, name->lex);
			struct_table_push_member(name->lex, dd_variable_type_convert(type->lex), type->lex);
		}
		else {
			//printf("function: %s %s\n", type->lex, name->lex);

			int parentDepth = struct_table_is_member_parent(structIndex, name->lex);
			//printf("var: %s %s\n", type->lex, name->lex);
			//printf("parent depth: %d\n", parentDepth);

			// override function
			if (parentDepth >= 0) {
				child->value = 1;
			}
			// new function
			else {
			}
			struct_table_push_member(name->lex, DD_VARIABLE_TYPE_FUNCTION, 0);
		}
	}

	/* scan definitions
	 * if a function was defined in any of the subclasses, mark is
	 * 	as an override
	 */

	struct ast_node *classDefinition = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(classDefinition, "class");
	ast_child_add(classDefinition, classname);
	ast_child_add(classDefinition, subclassname);
	ast_child_add(classDefinition, definitions);
	return classDefinition;
}

static struct ast_node *expect_command_definition() {

	struct ast_node *definition = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(definition, "def");

	// get type
	struct ast_node *type = expect_identifier();

	/*
	// check if primitive or struct
	if (!dd_variable_type_isPrimitiveType(type->lex)) {
		semantic_error("unrecognized type '%s'", type->lex);
	}
	*/

	// get variable name
	struct ast_node *varname = expect_identifier();

	// add newly defined variable to symbol table
	struct entry *e = symtable_entryat(symtable_insert(varname->lex, SYMTABLE_VARIABLE));
	e->value = dd_variable_type_convert(type->lex);

	ast_child_add(definition, type);
	ast_child_add(definition, varname);
	return definition;
}

static struct ast_node *expect_int() {

	// confirm it's an integer
	if (lexer_getNextToken() != LEXER_TOKEN_INT) {
		semantic_error("expected integer instead of '%s'", lexer_getLexToken());
	}

	struct ast_node *integer = ast_create(AST_NUMBER, atoi(lexer_getLexToken()));
	return integer;
}

static struct ast_node *expect_float() {

	// confirm it's a float
	if (lexer_getNextToken() != LEXER_TOKEN_FLOAT) {
		semantic_error("expected float instead of '%s'", lexer_getLexToken());
	}

	struct ast_node *f = ast_create(AST_FLOAT, 0);
	f->fvalue = atof(lexer_getLexToken());
	return f;
}

static struct ast_node *expect_string() {

	// confirm it's a string
	if (lexer_getNextToken() != LEXER_TOKEN_STRING) {
		semantic_error("expected string instead of '%s'", lexer_getLexToken());
	}

	struct ast_node *str = ast_create(AST_STRING, 0);
	ast_addLex(str, lexer_getLexToken());
	return str;
}

static struct ast_node *expect_command_binaryOperation(const char *binaryOperationLex) {

	struct ast_node *binaryOperation = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(binaryOperation, binaryOperationLex);

	while (lexer_peek() != LEXER_TOKEN_COMMANDEND) {
		ast_child_add(binaryOperation, expect_command_arg());
	}

	return binaryOperation;
}

static struct ast_node *expect_identifier() {

	// confirm it's an identifier
	if (lexer_getNextToken() != LEXER_TOKEN_IDENTIFIER) {
		semantic_error("expected identifier instead of '%s'", lexer_getLexToken());
	}

	// generate ast node for it
	struct ast_node *identifier = ast_create(AST_IDENTIFIER, 0);
	ast_addLex(identifier, lexer_getLexToken());

	int symId = symtable_lookup(identifier->lex);
	if (symId >= 0) {
		struct entry *e = symtable_entryat(symId);
		identifier->isRef = e->isRef;
	}

	// does it have an array modifier?
	if (lexer_peek() == LEXER_TOKEN_ARRAYSTART) {
		lexer_getNextToken();
		struct ast_node *array = ast_create(AST_GROUP, 0);
		int token = lexer_peek();
		// integer as array modifier
		if (token == LEXER_TOKEN_INT) {
			ast_child_add(array, expect_int());
		}
		else
		// identifier as array modifier
		if (token == LEXER_TOKEN_IDENTIFIER) {
			ast_child_add(array, expect_identifier());
		}
		else
		// calculation as array modifier
		if (token == LEXER_TOKEN_COMMANDSTART) {
			lexer_getNextToken();
			while (lexer_peek() == LEXER_TOKEN_COMMANDSTART) {
				ast_child_add(array, expect_command());
			}
		}
		ast_child_add(identifier, array);

		// end of array
		if (lexer_getNextToken() != LEXER_TOKEN_ARRAYEND) {
			semantic_error("expected end of array");
		}
	}

	// it has a period (myclass.myvar)
	if (lexer_peek() == LEXER_TOKEN_PERIOD) {
		lexer_getNextToken();
		struct ast_node *child = expect_identifier();
		ast_child_add(identifier, child);
	}

	return identifier;
}

static struct ast_node *expect_command_arg() {

	int token = lexer_peek();
	if (token == LEXER_TOKEN_INT) {
		return expect_int();
	}
	else
	if (token == LEXER_TOKEN_FLOAT) {
		return expect_float();
	}
	else
	if (token == LEXER_TOKEN_STRING) {
		return expect_string();
	}
	else
	if (token == LEXER_TOKEN_COMMANDSTART) {
		return expect_command();
	}
	else
	if (token == LEXER_TOKEN_IDENTIFIER) {
		return expect_identifier();
	}
	else {
		lexer_getNextToken();
		semantic_error("expected command argument instead of '%s'", lexer_getLexToken());
	}

	return 0;
}

// looks for the structure of a command: (cmdname arg1 arg2 .. argN)
static struct ast_node *expect_command() {

	// confirm that a command is expected
	if (lexer_getNextToken() != LEXER_TOKEN_COMMANDSTART) {
		semantic_error("expected the start of a command '(', instead of '%s'", lexer_getLexToken());
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
		else
		if (strcmp(cmdname->lex, "class") == 0) {
			cmd = expect_command_classDefinition();
		}
		else
		if (strcmp(cmdname->lex, "group") == 0) {
			cmd = expect_command_group();
		}
		else
		if (strcmp(cmdname->lex, "function") == 0) {
			cmd = expect_command_functionDefinition();

			if (lexer_peek() == LEXER_TOKEN_COMMANDSTART) {
				// function statements
				ast_child_add(cmd, expect_command());
			}

		}
		else
		if (strcmp(cmdname->lex, "class_function") == 0) {
			cmd = expect_command_classFunction();
		}
		else
		if (strcmp(cmdname->lex, "=") == 0
		||  strcmp(cmdname->lex, "+") == 0
		||  strcmp(cmdname->lex, "-") == 0
		||  strcmp(cmdname->lex, "*") == 0
		||  strcmp(cmdname->lex, "/") == 0
		||  strcmp(cmdname->lex, "%") == 0
		||  strcmp(cmdname->lex, "&&") == 0
		||  strcmp(cmdname->lex, "||") == 0
		||  strcmp(cmdname->lex, "==") == 0
		||  strcmp(cmdname->lex, ">=") == 0
		||  strcmp(cmdname->lex, "<=") == 0
		||  strcmp(cmdname->lex, ">") == 0
		||  strcmp(cmdname->lex, "<") == 0) {
			cmd = expect_command_binaryOperation(cmdname->lex);
		}
		else
		if (strcmp(cmdname->lex, "echo") == 0) {
			cmd = expect_command_group();
			ast_addLex(cmd, "echo");
		}
		else
		if (strcmp(cmdname->lex, "if") == 0) {
			cmd = expect_command_if();
		}
		else
		if (strcmp(cmdname->lex, "include") == 0) {
			cmd = expect_command_include();
		}
		else {
			semantic_error("no rule to parse command '%s'", cmdname->lex);
		}
	}
	// custom command, make sure it exists
	else {
		/*
		if (symtable_lookup(cmdname->lex) == -1) {
			printf("unrecognized identifier: %s\n", cmdname->lex);
			exit(-1);
		}
		*/
		cmd = ast_create(0, 0);
		cmd->node_type = AST_COMMAND_CUSTOM;
		ast_child_add(cmd, cmdname);

		while (lexer_peek() != LEXER_TOKEN_COMMANDEND) {
			ast_child_add(cmd, expect_command_arg());
		}
	}

	// get the command's children
	if (lexer_getNextToken() != LEXER_TOKEN_COMMANDEND) {
		semantic_error("expected command end ')'");
	}

	return cmd;
}

void semanticAnalyser_convertToAst(struct ast_node *node, const char *filename) {

	struct_table_init();
	symtable_init();
	lexer_prepare(filename);

	struct ast_node *cmd;
	while (lexer_peek() == LEXER_TOKEN_COMMANDSTART) {
		cmd = expect_command();

		if (cmd->node_type == AST_INCLUDE) {
			lexer_addIncludedFile(cmd->lex);
		}
		else {
			ast_child_add(node, cmd);
		}
	}

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
