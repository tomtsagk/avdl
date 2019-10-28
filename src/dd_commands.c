#include "dd_commands.h"
#include "symtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const struct keyword_function keywords[] = {
	{"echo", 0},
	{"def", 0},
	{"=", 0},
	{"+", 0},
	{"-", 0},
	{"*", 0},
	{"/", 0},
	{"%", 0},
	{">=", 0},
	{"==", 0},
	{"<=", 0},
	{"&&", 0},
	{"||", 0},
	{"<", 0},
	{">", 0},
	{"group", 0},
	{"class", 0},
	{"function", 0},
	{"return", 0},
	{"array", 0},
	{"new", 0},
	{"if", 0},
	{"for", 0},
};
unsigned int keywords_total = sizeof(keywords) /sizeof(struct keyword_function);

struct ast_node *parse_command(struct ast_node *cmd_name, struct ast_node *opt_args) {

	struct entry *e = symtable_entryat(cmd_name->value);

	// if command has a custom way of parsing, do it
	for (unsigned int i = 0; i < keywords_total; i++) {
		if (strcmp(keywords[i].keyword, e->lexptr) == 0) {
			if (keywords[i].function) {
				keywords[i].function();
				return 0;
			}
			else {
				printf("error, command '%s' does not have a way to be parsed\n", e->lexptr);
				//exit(-1);
			}
		}
	}

	// default behaviour
	struct ast_node *ast = ast_create(AST_EMPTY, 0);
	return ast;
}
