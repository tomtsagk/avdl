#include "semantic_analyser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "struct_table.h"
#include "agc_commands.h"
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
static struct ast_node *expect_command_asset();
static struct ast_node *expect_command_for();
static void semantic_error(const char *msg, ...);

static struct ast_node *expect_command_for() {
	struct ast_node *definition = expect_command();
	struct ast_node *condition = expect_command();
	struct ast_node *step = expect_command();
	struct ast_node *statements = expect_command();

	struct ast_node *forcmd = ast_create(AST_COMMAND_NATIVE, 0);
	ast_addLex(forcmd, "for");

	ast_child_add(forcmd, definition);
	ast_child_add(forcmd, condition);
	ast_child_add(forcmd, step);
	ast_child_add(forcmd, statements);

	return forcmd;
}

static struct ast_node *expect_command_asset() {
	struct ast_node *asset = expect_string();

	// waste the type for now
	expect_identifier();

	return asset;
}

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
	e->value = struct_table_get_index(classname->lex);
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

static struct ast_node *getIdentifierArrayNode(struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);

		if (child->node_type == AST_GROUP) {
			return child;
		}
	}

	return 0;
}

static struct ast_node *expect_command_classDefinition() {

	struct ast_node *classname = expect_identifier();

	// subclass can be an identifier (name of the class) or `0` (no parent class)
	struct ast_node *subclassname;
	if (lexer_peek() == LEXER_TOKEN_IDENTIFIER) {
		subclassname = expect_identifier();
	}
	else {
		struct ast_node *n = expect_int();
		if (n->value != 0) {
			semantic_error("subclass can either be an identifier or '0'");
		}
		subclassname = 0;
	}

	symtable_push();
	struct ast_node *definitions = expect_command();
	symtable_pop();

	int structIndex = struct_table_push(classname->lex, subclassname->lex);

	for (int i = 0; i < definitions->children.elements; i++) {
		struct ast_node *child = dd_da_get(&definitions->children, i);
		struct ast_node *type = dd_da_get(&child->children, 0);
		struct ast_node *name = dd_da_get(&child->children, 1);

		if (strcmp(child->lex, "def") == 0) {

			struct ast_node *arrayNode = getIdentifierArrayNode(name);
			//printf("variable: %s %s\n", type->lex, name->lex);
			if (arrayNode) {

				if (arrayNode->children.elements == 0) {
					semantic_error("array definition should have a value");
				}

				struct ast_node *arrayNum = dd_da_get(&arrayNode->children, 0);

				if (arrayNum->node_type != AST_NUMBER) {
					semantic_error("array definition should only be a number");
				}
				struct_table_push_member_array(name->lex, dd_variable_type_convert(type->lex), type->lex, arrayNum->value);
			}
			else {
				struct_table_push_member(name->lex, dd_variable_type_convert(type->lex), type->lex);
			}
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

	if (lexer_peek() != LEXER_TOKEN_COMMANDEND) {
		struct ast_node *initialValue = expect_command_arg();
		ast_child_add(definition, initialValue);
	}

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

		// identifiers that own objects have to be in the symbol table
		if (symId < 0) {
			semantic_error("identifier '%s' not a known symbol", identifier->lex);
		}

		struct entry *e = symtable_entryat(symId);

		// identifiers that own objects have to be structs
		if (e->token != DD_VARIABLE_TYPE_STRUCT) {
			semantic_error("identifier '%s' not a struct, so it can't own objects");
		}

		//printf("name of struct: %s\n", struct_table_get_name(e->value));

		// add struct's members to new symbol table
		symtable_push();
		for (unsigned int j = 0; j < struct_table_get_member_total(e->value); j++) {
			/*
			printf("	member: %s %d %s\n",
				struct_table_get_member_name(e->value, j),
				struct_table_get_member_type(e->value, j),
				struct_table_get_member_nametype(e->value, j)
			);
			*/

			struct entry *e2 = symtable_entryat(symtable_insert(
				struct_table_get_member_name(e->value, j),
				struct_table_get_member_type(e->value, j))
			);
			if (struct_table_get_member_type(e->value, j) == DD_VARIABLE_TYPE_STRUCT) {
				e2->value = struct_table_get_index(struct_table_get_member_nametype(e->value, j));
			}
		}

		// get the owned object's name
		struct ast_node *child = expect_identifier();

		// child not inside symbol table - possibly belongs to a parent class
		int childSymId = symtable_lookup(child->lex);
		if (childSymId < 0) {
			int parentDepth = struct_table_is_member_parent(e->value, child->lex);

			// child not part of that struct, or any of its parent classes
			if (parentDepth < 0) {
				semantic_error("variable '%s' of class '%s' does not own object '%s'",
					identifier->lex, struct_table_get_name(e->value), child->lex
				);
			}
			else
			// child direct child of that class, but not in symbol table, this should never happen
			if (parentDepth == 0) {
				semantic_error("variable '%s' of class '%s' owns object '%s', but couldn't be found in the symbol table",
					identifier->lex, struct_table_get_name(e->value), child->lex
				);
			}

			// add "parent" children in ast, for each parent class depth level
			struct ast_node *lastChild = identifier;
			for (int i = 0; i < parentDepth; i++) {
				struct ast_node *parentChild = ast_create(AST_IDENTIFIER, 0);
				ast_addLex(parentChild, "parent");
				int childIndex = ast_child_add(lastChild, parentChild);
				lastChild = dd_da_get(&lastChild->children, childIndex);
			}

			// finally add the found child on the last "parent" node added
			ast_child_add(lastChild, child);
		}
		// child is directly owned by this struct, just add it as child
		else {
			if (struct_table_get_member_type(e->value, struct_table_get_member(e->value, child->lex)) == DD_VARIABLE_TYPE_STRUCT) {
				child->value = DD_VARIABLE_TYPE_STRUCT;
			}
			ast_child_add(identifier, child);
		}

		symtable_pop();
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
	if (agc_commands_isNative(cmdname->lex)) {

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
		else
		if (strcmp(cmdname->lex, "asset") == 0) {
			cmd = expect_command_asset();
		}
		else
		if (strcmp(cmdname->lex, "for") == 0) {
			cmd = expect_command_for();
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
