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
static struct ast_node *parse_through(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_def(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_array(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args);
static struct ast_node *parse_class(struct ast_node *cmd_name, struct ast_node *opt_args);

// native keywords
const struct keyword_function keywords[] = {
	{"echo", parse_through},
	{"def", parse_def},
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
	{"group", parse_group},
	{"class", parse_class},
	{"class_function", parse_through},
	{"function", parse_through},
	{"return", parse_through},
	{"array", parse_array},
	{"new", parse_through},
	{"if", parse_through},
	{"for", parse_through},
	{"asset", parse_through},
	{"extern", parse_through},
};
unsigned int keywords_total = sizeof(keywords) /sizeof(struct keyword_function);

struct ast_node *parse_command(struct ast_node *cmd_name, struct ast_node *opt_args) {

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

	// not a native command, can't be parsed
	return 0;
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

	return parse_through(cmd_name, opt_args);
}

static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args) {
	struct ast_node *n = parse_through(cmd_name, opt_args);
	n->node_type = AST_GROUP;
	return n;
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

	struct ast_node *varname = dd_da_get(&opt_args->children, 1);
	struct entry *evarname = symtable_entryat(varname->value);

	struct ast_node *vartype = dd_da_get(&opt_args->children, 0);
	struct entry *evartype = symtable_entryat(vartype->value);

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
}

static struct ast_node *parse_through(struct ast_node *cmd_name, struct ast_node *opt_args) {
	opt_args->node_type = AST_COMMAND_NATIVE;
	opt_args->value = cmd_name->value;
	opt_args->isIncluded = include_stack_ptr != 0;
	return opt_args;
}
