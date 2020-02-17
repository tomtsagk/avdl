#include "parser.h"
#include "stdlib.h"
#include "stdio.h"
#include "yacc.tab.h"
#include "symtable.h"
#include <string.h>
#include "file_op.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DD_BUFFER_SIZE 1000
static char buffer[DD_BUFFER_SIZE];

// flags
static int has_semicolon = 1;

void print_node(FILE *fd, struct ast_node *n);

// file descriptor for global data
FILE *fd_global;

struct command_translation {
	char *command;
	char *translation;
};

static void print_command(FILE *fd, struct ast_node *command);
static void print_echo(FILE *fd, struct ast_node *command);
static void print_definition(FILE *fd, struct ast_node *command);
static void print_operator_binary(FILE *fd, struct ast_node *command);
static void print_number(FILE *fd, struct ast_node *command);
static void print_float(FILE *fd, struct ast_node *command);
static void print_function(FILE *fd, struct ast_node *command);
static void print_class(FILE *fd, struct ast_node *command);
static void print_return(FILE *fd, struct ast_node *command);
static void print_array(FILE *fd, struct ast_node *command);
static void print_function_call(FILE *fd, struct ast_node *command);
static void print_identifier(FILE *fd, struct ast_node *command);
static void print_new(FILE *fd, struct ast_node *command);
static void print_if(FILE *fd, struct ast_node *command);

void print_if(FILE *fd, struct ast_node *command) {

	unsigned int cchild = 0;
	while (cchild < command->children.elements) {

		if (cchild != 0) {
			fprintf(fd, "else ");
		}

		if (command->children.elements -cchild >= 2) {
			fprintf(fd, "if (");
			has_semicolon = 0;
			print_node(fd, dd_da_get(&command->children, cchild));
			has_semicolon = 1;
			fprintf(fd, ")");
			cchild++;
		}
		fprintf(fd, " {\n");
		print_node(fd, dd_da_get(&command->children, cchild));
		fprintf(fd, "}\n");
		cchild++;
	}
}

void print_new(FILE *fd, struct ast_node *command) {
	fprintf(fd, "new ");
	print_node(fd, dd_da_get(&command->children, 0));
	fprintf(fd, "(");
	has_semicolon = 0;
	for (unsigned int i = 1; i < command->children.elements; i++) {
		if (i != 1) {
			fprintf(fd, ", ");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	has_semicolon = 1;
	fprintf(fd, ")");
}

void print_function_call(FILE *fd, struct ast_node *command) {
	print_node(fd, dd_da_get(&command->children, 0));
	fprintf(fd, "(");

	int prev_semi = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 1; i < command->children.elements; i++) {
		if (i != 1) {
			fprintf(fd, ", ");
		}
		struct ast_node *child = dd_da_get(&command->children, i);
		print_node(fd, child);
	}
	has_semicolon = prev_semi;

	fprintf(fd, ")");
	if (has_semicolon) {
		fprintf(fd, ";\n");
	}

}

void print_number(FILE *fd, struct ast_node *command) {
	fprintf(fd, "%d", command->value);
}

void print_float(FILE *fd, struct ast_node *command) {
	fprintf(fd, "%f", command->fvalue);
}

void print_operator_binary(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);
	struct ast_node *child1 = dd_da_get(&command->children, 0);

	if (strcmp(e->lexptr, "=") != 0) {
		fprintf(fd, "(");
	}
	print_node(fd, child1);

	int temp_semicolon = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 1; i < command->children.elements; i++) {
		fprintf(fd, " %s ", e->lexptr);
		struct ast_node *child2 = dd_da_get(&command->children, i);
		print_node(fd, child2);
	}
	has_semicolon = temp_semicolon;
	if (strcmp(e->lexptr, "=") != 0) {
		fprintf(fd, ")");
	}
}

void print_definition(FILE *fd, struct ast_node *command) {
	struct ast_node *varname = dd_da_get(&command->children, 1);

	int is_var = 1;

	// if definition is direct child of class, use `this.VARNAME` instead of `var VARNAME`
	struct ast_node *parent;
	if (command->parent) {
		parent = command->parent;
		if (parent->node_type == AST_GROUP
		&&  parent->parent->node_type == AST_COMMAND_NATIVE) {
			struct entry *e = symtable_entryat(parent->parent->value);
			if (strcmp(e->lexptr, "class") == 0) {
				is_var = 0;
				fprintf(fd, "this.");
			}
		}
	}

	// if definition has children (like `this.x`) don't use `var`
	if (is_var) {
		fprintf(fd, "var ");
	}

	print_node(fd, varname);
	if (command->children.elements >= 3) {
		fprintf(fd, " = ");
		print_node(fd, dd_da_get(&command->children, 2));
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
	if (strcmp(e->lexptr, "def") == 0) {
		print_definition(fd, command);
	}
	else
	if (strcmp(e->lexptr, "new") == 0) {
		print_new(fd, command);
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
	||  strcmp(e->lexptr, "/") == 0
	||  strcmp(e->lexptr, ">=") == 0
	||  strcmp(e->lexptr, "==") == 0
	||  strcmp(e->lexptr, "<=") == 0
	||  strcmp(e->lexptr, "&&") == 0
	||  strcmp(e->lexptr, "||") == 0
	||  strcmp(e->lexptr, "<") == 0
	||  strcmp(e->lexptr, ">") == 0) {
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
	if (strcmp(e->lexptr, "return") == 0) {
		print_return(fd, command);
	}
	else
	if (strcmp(e->lexptr, "array") == 0) {
		print_array(fd, command);
	}
	else
	if (strcmp(e->lexptr, "if") == 0) {
		print_if(fd, command);
	}
}

void print_echo(FILE *fd, struct ast_node *command) {
	fprintf(fd, "console.log(");
	int temp_semicolon = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 0; i < command->children.elements; i++) {
		if (i > 0) {
			fprintf(fd, " +");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	has_semicolon = temp_semicolon;
	fprintf(fd, ");\n");
}

void print_function(FILE *fd, struct ast_node *command) {
	fprintf(fd, "this.");

	//name
	struct ast_node *name = dd_da_get(&command->children, 0);
	print_node(fd, name);

	fprintf(fd, " = function(");

	//arguments
	has_semicolon = 0;
	struct ast_node *arg = dd_da_get(&command->children, 1);
	print_node(fd, arg);
	has_semicolon = 1;

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

	int cchild = 1;

	// subclass
	struct ast_node *subclass = dd_da_get(&command->children, 1);
	if (subclass->node_type == AST_IDENTIFIER) {
		struct entry *subentry = symtable_entryat(subclass->value);
		fprintf(fd, "%s.call(this);\n", subentry->lexptr);
		cchild++;
	}

	//components
	struct ast_node *cmn = dd_da_get(&command->children, cchild);
	print_node(fd, cmn);

	fprintf(fd, "}\n");
}

void print_return(FILE *fd, struct ast_node *command) {
	fprintf(fd, "return ");
	for (unsigned int i = 0; i < command->children.elements; i++) {
		print_node(fd, dd_da_get(&command->children, i));
	}
	fprintf(fd, ";\n");
}

void print_array(FILE *fd, struct ast_node *command) {
	fprintf(fd, "[\n");
	for (unsigned int i = 0; i < command->children.elements; i++) {
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

	for (unsigned int i = 0; i < command->children.elements; i++) {
		fprintf(fd, ".");
		print_node(fd, dd_da_get(&command->children, i));
	}
}

void print_node(FILE *fd, struct ast_node *n) {
	switch (n->node_type) {
		case AST_GAME:
		case AST_GROUP:
			for (unsigned int i = 0; i < n->children.elements; i++) {
				print_node(fd, dd_da_get(&n->children, i));
			}
			break;
		case AST_COMMAND_NATIVE: {
			print_command(fd, n);
			break;
		}
		case AST_COMMAND_CUSTOM: {
			print_function_call(fd, n);
			break;
		}
		case AST_NUMBER: {
			print_number(fd, n);
			break;
		}
		case AST_FLOAT: {
			print_float(fd, n);
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
		case AST_EMPTY: break;
		case AST_INCLUDE: break;
	}
}

#ifndef INSTALL_LOCATION
#define INSTALL_LOCATION "/usr"
#endif

#ifndef PROJECT_NAME
#define PROJECT_NAME "ddlang"
#endif

// responsible for creating a file and translating ast to target language
void parse_javascript(const char *filename, struct ast_node *n) {
	dir_create("build");

	fd_global = fopen(filename, "w");
	if (!fd_global) {
		printf("unable to open '%s': %s\n", filename, strerror(errno));
		return;
	}
	fprintf(fd_global, "var eng = new engine();\n");
	print_node(fd_global, n);
	fprintf(fd_global, "eng.setWorld(world_main);\n");
	fclose(fd_global);

	int dir = open("build", O_DIRECTORY);
	if (!dir) {
		printf("error opening `build/`: %s\n", strerror(errno));
	}
	sprintf(buffer, "%s/share/%s/dd_pixi_engine.js", INSTALL_LOCATION, PROJECT_NAME);
	file_copy_at(0, buffer, dir, "dd_pixi_engine.js", 0);
	sprintf(buffer, "%s/share/%s/index.html", INSTALL_LOCATION, PROJECT_NAME);
	file_copy_at(0, buffer, dir, "index.html", 0);
	sprintf(buffer, "%s/share/%s/pixi.min.js", INSTALL_LOCATION, PROJECT_NAME);
	file_copy_at(0, buffer, dir, "pixi.min.js", 0);

	dir_create("build/images");
	dir_copy_recursive(0, "images", dir, "images");
	close(dir);
}
