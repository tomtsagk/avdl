#include "dd_commands.h"
#include "symtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "struct_table.h"

// keywords for primitive data
const char *primitive_keywords[] = {
	"int",
	"float",
	"char",
};
unsigned int primitive_keywords_count = sizeof(primitive_keywords) /sizeof(char *);

extern int include_stack_ptr;

// how to parse commands
//static struct ast_node *parse_through(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_through(struct ast_node *cmd);
static struct ast_node *parse_def(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_array(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_class(struct ast_node *cmd_name, struct ast_node *opt_args);

static void validate_class_function(struct ast_node *node);
static void validate_class(struct ast_node *node);
static void validateCommand(struct ast_node *node);

// native keywords
const struct keyword_function keywords[] = {
	{"echo", parse_through},
	//{"def", parse_def},
	{"def", parse_through},
	{"=", parse_through},
	{"+", parse_through},
	{"-", parse_through},

	{"*", parse_through},
	{"/", parse_through},
	{"%", parse_through},
	{">=", parse_through},
	{"==", parse_through},

	{"<=", parse_through},
	{"&&", parse_through},
	{"||", parse_through},
	{"<", parse_through},
	{">", parse_through},

	//{"group", parse_group},
	{"group", parse_through},
	//{"class", parse_class},
	{"class", parse_through},
	{"class_function", parse_through},
	{"function", parse_through},
	{"return", parse_through},

	//{"array", parse_array},
	{"array", parse_through},
	{"new", parse_through},
	{"if", parse_through},
	{"for", parse_through},
	{"asset", parse_through},

	{"extern", parse_through},
	{"multistring", parse_through},
};
unsigned int keywords_total = sizeof(keywords) /sizeof(struct keyword_function);

struct ast_node *parse_command(struct ast_node *cmd_name, struct ast_node *opt_args) {
/*

	struct entry *e = symtable_entryat(cmd_name->value);

	// if command has a custom way of parsing, do it
	for (unsigned int i = 0; i < keywords_total; i++) {
		if (strcmp(keywords[i].keyword, e->lexptr) == 0) {
			if (keywords[i].function) {
				return keywords[i].function(cmd_name, opt_args);
			}
			else {
				printf("error: command '%s' does not have a way to be parsed\n", e->lexptr);
				exit(-1);
			}
		}
	}
	*/

	// not a native command, can't be parsed
	return 0;
}

struct ast_node *parse_command2(struct ast_node *cmd) {

	struct entry *e = symtable_entryat(cmd->value);

	// if command has a custom way of parsing, do it
	for (unsigned int i = 0; i < keywords_total; i++) {
		if (strcmp(keywords[i].keyword, e->lexptr) == 0) {
			if (keywords[i].function) {
				return keywords[i].function(cmd);
			}
			else {
				printf("error: command '%s' does not have a way to be parsed\n", e->lexptr);
				exit(-1);
			}
		}
	}

	// not a native command, can't be parsed
	return cmd;
}

static struct ast_node *parse_class(struct ast_node *cmd_name, struct ast_node *opt_args) {

	struct entry *e = symtable_entryat(cmd_name->value);

	struct ast_node *classname = dd_da_get(&opt_args->children, 0);
	struct entry *eclassname = symtable_entryat(classname->value);

	// check if there is a parent
	struct ast_node *parentname = dd_da_get(&opt_args->children, 1);
	int struct_index;
	if (parentname->node_type == AST_IDENTIFIER) {
		struct entry *eparentname = symtable_entryat(parentname->value);
		struct_index = struct_table_push(eclassname->lexptr, eparentname->lexptr);
	}
	else {
		struct_index = struct_table_push(eclassname->lexptr, 0);
	}
	e->value = struct_index;

	// find all definitions
	struct ast_node *statements = dd_da_get(&opt_args->children, 2);
	for (unsigned int i = 0; i < statements->children.elements; i++) {

		// native command
		struct ast_node *statement = dd_da_get(&statements->children, i);
		if (statement->node_type == AST_COMMAND_NATIVE) {
			struct entry *estatement = symtable_entryat(statement->value);

			int isFunction = strcmp(estatement->lexptr, "function") == 0;
			if (strcmp(estatement->lexptr, "def") == 0
			||  isFunction) {
				struct ast_node *vartype = 0;
				struct entry *evartype = 0;
				int arrayCount = 1;
				if (!isFunction) {
					vartype = dd_da_get(&statement->children, 0);
					evartype = symtable_entryat(vartype->value);
					arrayCount = vartype->arraySize;
				}

				int varnameIndex = isFunction ? 0 : 1;

				struct ast_node *varname = dd_da_get(&statement->children, varnameIndex);
				struct entry *evarname = symtable_entryat(varname->value);

				// ignore constructor
				if (isFunction && strcmp(evarname->lexptr, "create") == 0) continue;

				char *nametype = 0;
				int type = 0;
				if (isFunction) {
					type = DD_VARIABLE_TYPE_FUNCTION;
				}
				else
				if (strcmp(evartype->lexptr, "int") == 0) {
					type = DD_VARIABLE_TYPE_INT;
				}
				else
				if (strcmp(evartype->lexptr, "float") == 0) {
					type = DD_VARIABLE_TYPE_FLOAT;
				}
				else
				if (strcmp(evartype->lexptr, "string") == 0) {
					type = DD_VARIABLE_TYPE_STRING;
				}
				else
				if (strcmp(evartype->lexptr, "char") == 0) {
					type = DD_VARIABLE_TYPE_CHAR;
				}
				else {
					type = DD_VARIABLE_TYPE_STRUCT;
					nametype = evartype->lexptr;
				}

				struct_table_push_member_array(evarname->lexptr, type, nametype, arrayCount);
			}
			if (strcmp(estatement->lexptr, "function") == 0) {
				struct ast_node *varname = dd_da_get(&statement->children, 0);
				struct entry *evarname = symtable_entryat(varname->value);

				// only add it to the struct table if not member of any of its parents
				if (struct_table_is_member_parent(
					struct_table_get_index(eclassname->lexptr),
					evarname->lexptr) == -1) {

					struct_table_push_member(evarname->lexptr, 0, 0);
				}
			}
		}
	}

	//return parse_through(cmd_name, opt_args);
	return 0;
}

static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args) {
	return 0;
	/*
	struct ast_node *n = parse_through(cmd_name, opt_args);
	n->node_type = AST_GROUP;
	return n;
	*/
}

static struct ast_node *parse_array(struct ast_node *cmd_name, struct ast_node *opt_args) {
	(void) cmd_name;
	struct ast_node *type = dd_da_get(&opt_args->children, 0);
	struct ast_node *amount = dd_da_get(&opt_args->children, 1);
	struct ast_node *n = ast_create(AST_IDENTIFIER, type->value);
	n->arraySize = amount->value;
	return n;
}

static struct ast_node *parse_def(struct ast_node *cmd_name, struct ast_node *opt_args) {
	return 0;
	/*

	struct ast_node *varname = dd_da_get(&opt_args->children, 1);
	struct entry *evarname = symtable_entryat(varname->value);

	struct ast_node *vartype = dd_da_get(&opt_args->children, 0);
	struct entry *evartype = symtable_entryat(vartype->value);

	// check if type is recognized
	evarname->varType = dd_variable_type_convert(evartype->lexptr);
	if (evarname->varType == DD_VARIABLE_TYPE_UNKNOWN) {
		printf("avdl: unrecognized type '%s'\n", evartype->lexptr);
		exit(-1);
	}

	int isValid = 0;

	// check if primitive type
	for (unsigned int i = 0; i < primitive_keywords_count; i++) {
		if (strcmp(primitive_keywords[i], evartype->lexptr) == 0) {
			isValid = 1;
			break;
		}
	}

	// check if struct type
	if (struct_table_get_index(evartype->lexptr) >= 0) {
		isValid = 1;
	}

	// check if valid type was entered
	if (!isValid) {
		printf("avdl error: declared invalid type: %s\n", evartype->lexptr);
		exit(-1);
	}

	evarname->scope = evartype->lexptr;
	return parse_through(cmd_name, opt_args);
	*/
}

//static struct ast_node *parse_through(struct ast_node *cmd_name, struct ast_node *opt_args) {
static struct ast_node *parse_through(struct ast_node *cmd) {
	return cmd;
	/*
	opt_args->node_type = AST_COMMAND_NATIVE;
	opt_args->value = cmd_name->value;
	opt_args->isIncluded = include_stack_ptr != 0;
	return opt_args;
	*/
}

static int scope;
static void validate_identifier(struct ast_node *node) {
	//printf("validating identifier: %s\n", node->lex);
	//printf("children: %d\n", node->children.elements);
	//printf("scope: %d\n", scope);
	if (strcmp(node->lex, "this") == 0
	&& node->children.elements > 0
	&& scope >= 0
	) {
		//printf("struct: %s\n", struct_table_get_name(scope));
		struct ast_node *child = dd_da_get(&node->children, 0);
		int parentCount = struct_table_is_member_parent(scope, child->lex);
		//printf("parent count: %d\n", parentCount);
		for (int i = 0; i < parentCount; i++) {
			struct ast_node *parentNode = ast_create(AST_IDENTIFIER, 0);
			ast_addLex(parentNode, "parent");
			ast_child_add_first(node, parentNode);
		}
	}
}

static void validateCustomCommand(struct ast_node *node) {

	struct ast_node *cmdname = dd_da_get(&node->children, 0);
	for (int i = 1; i < node->children.elements; i++) {
		struct ast_node *child = dd_da_get(&node->children, i);

		/*
		printf("validating: %s\n", child->lex);
		int id = symtable_lookup(child->lex);
		printf("id: %d\n", id);
		*/
		/*
		 * check symbol table
		 * if child is type struct, reference it
		 */
	}
}

static void validateCommand(struct ast_node *node) {

	if (node->node_type == AST_GROUP
	||  node->node_type == AST_GAME) {
		for (int i = 0; i < node->children.elements; i++) {
			struct ast_node *child = dd_da_get(&node->children, i);
			validateCommand(child);
		}
	}
	else
	if (node->node_type == AST_COMMAND_NATIVE) {
		struct ast_node *childname = dd_da_get(&node->children, 0);
		if (strcmp(childname->lex, "class") == 0) {
			validate_class(node);
		}
		else
		if (strcmp(childname->lex, "class_function") == 0) {
			validate_class_function(node);
		}
		else
		if (strcmp(childname->lex, "group") == 0) {
			//validate_class_function(node);
			for (int i = 0; i < node->children.elements; i++) {
				struct ast_node *child = dd_da_get(&node->children, i);
				validateCommand(child);
			}
		}
		else {
			for (int i = 0; i < node->children.elements; i++) {
				struct ast_node *child = dd_da_get(&node->children, i);
				validateCommand(child);
			}
		}
	}
	else
	if (node->node_type == AST_COMMAND_CUSTOM) {
		validateCustomCommand(node);
	}
	else
	if (node->node_type == AST_IDENTIFIER) {
		validate_identifier(node);
	}
	else {
		for (int i = 0; i < node->children.elements; i++) {
			struct ast_node *child = dd_da_get(&node->children, i);
			validateCommand(child);
		}
	}
}

static void validate_statement(struct ast_node *node) {
	if (node->node_type == AST_IDENTIFIER) {
		validate_identifier(node);
	}
	else {
		for (int i = 0; i < node->children.elements; i++) {
			struct ast_node *child = dd_da_get(&node->children, i);
			validate_statement(child);
		}
	}
}

static void validate_class_function(struct ast_node *node) {
	struct ast_node *classname = dd_da_get(&node->children, 1);
	//struct ast_node *funcname = dd_da_get(&node->children, 2);
	//struct ast_node *args = dd_da_get(&node->children, 3);
	struct ast_node *statements = dd_da_get(&node->children, 4);

	//printf("func: %s args:\n", funcname->lex);
	//ast_print(args);

	scope = struct_table_get_index(classname->lex);

	// make 'this' visible that points to an object of the function's class
	symtable_push();
	int entryId = symtable_insert("this", DD_SYMTABLE_STRUCT);
	struct entry *e = symtable_entryat(entryId);
	e->value = scope;

	//symtable_print();

	validateCommand(statements);
	/*
	for (int i = 0; i < statements->children.elements; i++) {
		struct ast_node *child = dd_da_get(&statements->children, i);
		//validate_statement(child);
		validateCommand(child);
	}
	*/
	symtable_pop();
	scope = 0;
}

static void validate_class(struct ast_node *class) {

	// expected children
	struct ast_node *classname = dd_da_get(&class->children, 1);
	struct ast_node *parentname = dd_da_get(&class->children, 2);
	struct ast_node *statements = dd_da_get(&class->children, 3);

	// get parent (if any)
	const char *parentLex = 0;
	if (parentname->node_type == AST_IDENTIFIER) {
		parentLex = parentname->lex;
	}

	/*
	printf("validating class: %s\n", classname->lex);
	if (parentLex) {
		printf("with parent: %s\n", parentLex);
	}
	*/

	// class already exists
	if (struct_table_exists(classname->lex)) {
		printf("avdl error: class '%s' already exists\n", classname->lex);
		exit(-1);
	}

	// add class to struct table
	struct_table_push(classname->lex, parentLex);

	// find all definition of variables and functions
	for (unsigned int i = 1; i < statements->children.elements; i++) {
		struct ast_node *statement = dd_da_get(&statements->children, i);

		if (statement->node_type != AST_COMMAND_NATIVE) {
			printf("statement in class definition not a definition or function\n");
			printf("avdl error: in class '%s': statement is not a native definition or function\n",
				classname->lex
			);
			exit(-1);
		}

		struct ast_node *statementname = dd_da_get(&statement->children, 0);

		// definitions
		if (strcmp(statementname->lex, "def") == 0) {
			struct ast_node *deftype = dd_da_get(&statement->children, 1);
			struct ast_node *defname = dd_da_get(&statement->children, 2);

			// find the variable's type, if it's primitive or not
			int type = 0;
			char *nametype = 0;
			if (strcmp(deftype->lex, "int") == 0
			||  strcmp(deftype->lex, "string") == 0
			||  strcmp(deftype->lex, "float") == 0) {
				type = DD_VARIABLE_TYPE_UNKNOWN;
			}
			else {
				type = DD_VARIABLE_TYPE_STRUCT;
				nametype = deftype->lex;
			}

			// calc array
			int arrayCount = 0;
			if (defname->children.elements > 0) {
				struct ast_node *lastChild = dd_da_get(&defname->children, defname->children.elements-1);
				if (lastChild->arraySize > 0) {
					arrayCount = lastChild->arraySize;
				}
			}

			struct_table_push_member_array(defname->lex, type, nametype, arrayCount);
		}
		else
		// functions
		if (strcmp(statementname->lex, "function") == 0) {
			struct ast_node *funcname = dd_da_get(&statement->children, 1);
			// only add it to the struct table if not member of any of its parents
			//if (struct_table_is_member_parent(
			//struct_table_get_index(classname->lex), funcname->lex) == -1) {
				struct_table_push_member(funcname->lex, DD_VARIABLE_TYPE_FUNCTION, 0);
			//}
		}
		// only definion of variables and functions are allowed, anything else is an error
		else {
			printf("avdl error: in class '%s': statement is not a definition or function: '%s'\n",
				classname->lex, statement->lex
			);
			exit(-1);
		}
	}

//	// find all definitions
//	struct ast_node *statements = dd_da_get(&opt_args->children, 2);
//	for (unsigned int i = 0; i < statements->children.elements; i++) {
//
//		// native command
//		struct ast_node *statement = dd_da_get(&statements->children, i);
//		if (statement->node_type == AST_COMMAND_NATIVE) {
//			struct entry *estatement = symtable_entryat(statement->value);
//
//			int isFunction = strcmp(estatement->lexptr, "function") == 0;
//			if (strcmp(estatement->lexptr, "def") == 0
//			||  isFunction) {
//				struct ast_node *vartype = 0;
//				struct entry *evartype = 0;
//				int arrayCount = 1;
//				if (!isFunction) {
//					vartype = dd_da_get(&statement->children, 0);
//					evartype = symtable_entryat(vartype->value);
//					arrayCount = vartype->arraySize;
//				}
//
//				int varnameIndex = isFunction ? 0 : 1;
//
//				struct ast_node *varname = dd_da_get(&statement->children, varnameIndex);
//				struct entry *evarname = symtable_entryat(varname->value);
//
//				// ignore constructor
//				if (isFunction && strcmp(evarname->lexptr, "create") == 0) continue;
//
//				char *nametype = 0;
//				int type = 0;
//				if (isFunction) {
//					type = DD_VARIABLE_TYPE_FUNCTION;
//				}
//				else
//				if (strcmp(evartype->lexptr, "int") == 0) {
//					type = DD_VARIABLE_TYPE_INT;
//				}
//				else
//				if (strcmp(evartype->lexptr, "float") == 0) {
//					type = DD_VARIABLE_TYPE_FLOAT;
//				}
//				else
//				if (strcmp(evartype->lexptr, "string") == 0) {
//					type = DD_VARIABLE_TYPE_STRING;
//				}
//				else
//				if (strcmp(evartype->lexptr, "char") == 0) {
//					type = DD_VARIABLE_TYPE_CHAR;
//				}
//				else {
//					type = DD_VARIABLE_TYPE_STRUCT;
//					nametype = evartype->lexptr;
//				}
//
//				struct_table_push_member_array(evarname->lexptr, type, nametype, arrayCount);
//			}
}


void dd_commands_validate(struct ast_node *node) {

	scope = -1;
	struct_table_init();

	// initial symbols
	symtable_init();

	// keywords
	for (unsigned int i = 0; i < keywords_total; i++) {
		symtable_insert(keywords[i].keyword, DD_SYMTABLE_NATIVE);
	}
	symtable_print();

	if (node->node_type != AST_GAME) {
		printf("can only validate whole files for now\n");
		exit(-1);
	}

	validateCommand(node);

	/*
	for (int i = 0; i < node->children.elements; i++) {
		struct ast_node *child = dd_da_get(&node->children, i);

		if (child->node_type == AST_COMMAND_NATIVE) {
			struct ast_node *childname = dd_da_get(&child->children, 0);
			if (strcmp(childname->lex, "class") == 0) {
				validate_class(child);
			}
			else
			if (strcmp(childname->lex, "class_function") == 0) {
				validate_class_function(child);
			}
		}

	}
	*/
//
//	// if command has a custom way of parsing, do it
//	for (unsigned int i = 0; i < keywords_total; i++) {
//		if (strcmp(keywords[i].keyword, e->lexptr) == 0) {
///*
//			if (keywords[i].function) {
//				return keywords[i].function(cmd_name, opt_args);
//			}
//			else {
//				printf("error: command '%s' does not have a way to be parsed\n", e->lexptr);
//				exit(-1);
//			}
//	*/
//		}
//	}

	// not a native command, can't be parsed
	return;
}

int dd_commands_isNative(const char *cmdname) {
unsigned int keywords_total = sizeof(keywords) /sizeof(struct keyword_function);
	for (int i = 0; i < keywords_total; i++) {
		if (strcmp(keywords[i].keyword, cmdname) == 0) {
			return 1;
		}
	}
	return 0;
}
