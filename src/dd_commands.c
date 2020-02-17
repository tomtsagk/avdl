#include "dd_commands.h"
#include "symtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args);
//static struct ast_node *parse_def(struct ast_node *cmd_name, struct ast_node *opt_args);

const struct keyword_function keywords[] = {
	{"echo", parse_group},
	{"def", parse_group},
	{"=", parse_group},
	{"+", parse_group},
	{"-", parse_group},
	{"*", parse_group},
	{"/", parse_group},
	{"%", parse_group},
	{">=", parse_group},
	{"==", parse_group},
	{"<=", parse_group},
	{"&&", parse_group},
	{"||", parse_group},
	{"<", parse_group},
	{">", parse_group},
	{"group", parse_group},
	{"class", parse_group},
	{"class_function", parse_group},
	{"function", parse_group},
	{"return", parse_group},
	{"array", parse_group},
	{"new", parse_group},
	{"if", parse_group},
	{"for", parse_group},
	{"ref", parse_group},
	{"asset", parse_group},
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
				printf("error, command '%s' does not have a way to be parsed\n", e->lexptr);
				exit(-1);
			}
		}
	}

	// default behaviour
	struct ast_node *ast = ast_create(AST_EMPTY, 0);
	return ast;
}

// group is pass-through
static struct ast_node *parse_group(struct ast_node *cmd_name, struct ast_node *opt_args) {
	struct entry *e = symtable_entryat(cmd_name->value);
	if (strcmp(e->lexptr, "group") == 0) {
		opt_args->node_type = AST_GROUP;
	}
	return opt_args;
}

// definition - def <type> <name> <init-value?>
/*
static struct ast_node *parse_def(struct ast_node *cmd_name, struct ast_node *opt_args) {
	(void) cmd_name;
	return opt_args;
}
*/
