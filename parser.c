#include "parser.h"
#include "stdlib.h"
#include "stdio.h"
#include "yacc.tab.h"
#include "symtable.h"
#include <string.h>

#define DD_BUFFER_SIZE 1000
static char buffer[DD_BUFFER_SIZE];

void print_node(FILE *fd, struct ast_node *n);

// file descriptor for global data
FILE *fd_global;

struct command_translation {
	char *command;
	char *translation;
};

void print_command(FILE *fd, struct ast_node *command);
void print_echo(FILE *fd, struct ast_node *command);
void print_definition(FILE *fd, struct ast_node *command);
void print_operator_binary(FILE *fd, struct ast_node *command);
void print_number(FILE *fd, struct ast_node *command);
void print_function(FILE *fd, struct ast_node *command);
void print_class(FILE *fd, struct ast_node *command);
void print_sprite(FILE *fd, struct ast_node *command);
void print_return(FILE *fd, struct ast_node *command);
void print_array(FILE *fd, struct ast_node *command);

static struct command_translation translations[] = {
	{"sprite", "this.~0 = new PIXI.Sprite(PIXI.loader.resources[~1].texture);\n"
		"app.stage.addChild(this.~0);\n"},

};

void print_sprite(FILE *fd, struct ast_node *command) {
	fprintf(fd, "this.");

	//name
	struct ast_node *name = dd_da_get(&command->children, 0);
	print_node(fd, name);

	fprintf(fd, " = new PIXI.Sprite(PIXI.loader.resources[");

	//texture
	struct ast_node *tex = dd_da_get(&command->children, 1);
	print_node(fd, tex);

	fprintf(fd, "].texture);\n");
	fprintf(fd, "eng.app.stage.addChild(this.");
	print_node(fd, name);
	fprintf(fd, ");\n");
}

void print_number(FILE *fd, struct ast_node *command) {
	fprintf(fd, "%d", command->value);
}

void print_operator_binary(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);
	struct ast_node *child1 = dd_da_get(&command->children, 0);
	struct ast_node *child2 = dd_da_get(&command->children, 1);

	print_node(fd, child1);
	fprintf(fd, " %s ", e->lexptr);
	print_node(fd, child2);
}

void print_definition(FILE *fd, struct ast_node *command) {
	struct ast_node *varname = dd_da_get(&command->children, 0);
	struct entry *e = symtable_entryat(varname->value);
	fprintf(fd, "var %s", e->lexptr);
	if (command->children.elements >= 2) {
		fprintf(fd, " = ");
		print_node(fd, dd_da_get(&command->children, 1));
	}
	fprintf(fd, ";\n");
}

/* find out which command it is, and call the right function
 */
void print_command(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);
	if (strcmp(e->lexptr, "echo") == 0) {
		print_echo(fd, command);
	}
	else
	if (strcmp(e->lexptr, "int") == 0) {
		print_definition(fd, command);
	}
	else
	if (strcmp(e->lexptr, "=") == 0) {
		print_operator_binary(fd, command);
		fprintf(fd, ";\n");
	}
	else
	if (strcmp(e->lexptr, "+") == 0
	||  strcmp(e->lexptr, "-") == 0
	||  strcmp(e->lexptr, "*") == 0
	||  strcmp(e->lexptr, "/") == 0) {
		print_operator_binary(fd, command);
	}
	else
	if (strcmp(e->lexptr, "function") == 0) {
		print_function(fd, command);
	}
	else
	if (strcmp(e->lexptr, "class") == 0) {
		print_class(fd, command);
	}
	else
	if (strcmp(e->lexptr, "sprite") == 0) {
		print_sprite(fd, command);
	}
	else
	if (strcmp(e->lexptr, "return") == 0) {
		print_return(fd, command);
	}
	else
	if (strcmp(e->lexptr, "array") == 0) {
		print_array(fd, command);
	}
}

void print_echo(FILE *fd, struct ast_node *command) {
	fprintf(fd, "console.log(");
	for (int i = 0; i < command->children.elements; i++) {
		if (i > 0) {
			fprintf(fd, " +");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	fprintf(fd, ");\n");
}

void print_function(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);

	fprintf(fd, "this.");

	//name
	struct ast_node *name = dd_da_get(&command->children, 0);
	print_node(fd, name);

	fprintf(fd, " = function(");

	//arguments
	struct ast_node *arg = dd_da_get(&command->children, 1);
	print_node(fd, arg);

	fprintf(fd, ") {\n");

	//commands
	struct ast_node *cmd = dd_da_get(&command->children, 2);
	print_node(fd, cmd);

	fprintf(fd, "};\n");

}

void print_class(FILE *fd, struct ast_node *command) {
	fprintf(fd, "var ");

	// name
	struct ast_node *name = dd_da_get(&command->children, 0);
	print_node(fd, name);

	fprintf(fd, " = function() {\n");

	// subclass
	struct ast_node *subclass = dd_da_get(&command->children, 1);
	struct entry *subentry = symtable_entryat(subclass->value);
	fprintf(fd, "%s.call(this);\n", subentry->lexptr);

	//components
	struct ast_node *cmn = dd_da_get(&command->children, 2);
	print_node(fd, cmn);

	fprintf(fd, "}\n");
}

void print_return(FILE *fd, struct ast_node *command) {
	fprintf(fd, "return ");
	for (int i = 0; i < command->children.elements; i++) {
		print_node(fd, dd_da_get(&command->children, i));
	}
	fprintf(fd, ";\n");
}

void print_array(FILE *fd, struct ast_node *command) {
	fprintf(fd, "[\n");
	for (int i = 0; i < command->children.elements; i++) {
		if (i != 0) {
			fprintf(fd, ",\n");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	fprintf(fd, "]");
}

void print_identifier(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);
	fprintf(fd, "%s", e->lexptr);

	for (int i = 0; i < command->children.elements; i++) {
		fprintf(fd, ".");
		print_node(fd, dd_da_get(&command->children, i));
	}
}

void print_node(FILE *fd, struct ast_node *n) {
	switch (n->node_type) {
		case AST_GAME:
		case AST_GROUP:
			for (int i = 0; i < n->children.elements; i++) {
				print_node(fd, dd_da_get(&n->children, i));
			}
			break;
		case AST_COMMAND: {
			struct entry *e = symtable_entryat(n->value);
			print_command(fd, n);
			break;
		}
		case AST_NUMBER: {
			print_number(fd, n);
			break;
		}
		case AST_STRING: {
			struct entry *e = symtable_entryat(n->value);
			fprintf(fd, "'%s'", e->lexptr);
			break;
		}
		case AST_IDENTIFIER: {
			print_identifier(fd, n);
			break;
		}
	}
}

// responsible for creating a file and translating ast to target language
void parse_javascript(const char *filename, struct ast_node *n) {
	fd_global = fopen(filename, "w");
	fprintf(fd_global, "var eng;\n");
	print_node(fd_global, n);
	fprintf(fd_global, "eng = new engine(world_main);\n");
	fclose(fd_global);
}
