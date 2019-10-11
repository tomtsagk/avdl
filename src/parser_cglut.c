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
#include "struct_table.h"

#define DD_BUFFER_SIZE 1000
static char buffer[DD_BUFFER_SIZE];

// flags
static int has_semicolon = 1;

// file descriptor for global data
FILE *fd_global;

int scope = -1;

static void print_node(FILE *fd, struct ast_node *n);

/*
struct command_translation {
	char *command;
	char *translation;
};
*/

static void print_command(FILE *fd, struct ast_node *command);
static void print_class(FILE *fd, struct ast_node *command);
static void print_number(FILE *fd, struct ast_node *command);
static void print_float(FILE *fd, struct ast_node *command);
static void print_identifier(FILE *fd, struct ast_node *command);
static void print_definition(FILE *fd, struct ast_node *command);
static void print_operator_binary(FILE *fd, struct ast_node *command);
static void print_function_call(FILE *fd, struct ast_node *command);
/*
static void print_echo(FILE *fd, struct ast_node *command);
static void print_function(FILE *fd, struct ast_node *command);
static void print_return(FILE *fd, struct ast_node *command);
static void print_array(FILE *fd, struct ast_node *command);
static void print_new(FILE *fd, struct ast_node *command);
static void print_if(FILE *fd, struct ast_node *command);
*/
/*

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
*/

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
	struct ast_node *vartype = dd_da_get(&command->children, 0);
	if (vartype->node_type == AST_IDENTIFIER) {
		struct entry *e = symtable_entryat(vartype->value);
		if (strcmp(e->lexptr, "float") != 0
		&&  strcmp(e->lexptr, "int") != 0) {
			fprintf(fd, "struct ");
		}
	}
	struct entry *type_entry = symtable_entryat(vartype->value);
	fprintf(fd, "%s ", type_entry->lexptr);

	struct ast_node *varname = dd_da_get(&command->children, 1);

	// if definition is direct child of class, use `this.VARNAME` instead of `var VARNAME`
	/*
	struct ast_node *parent;
	if (command->parent) {
		parent = command->parent;
		if (parent->node_type == AST_GROUP
		&&  parent->parent->node_type == AST_COMMAND_NATIVE) {
			struct entry *e = symtable_entryat(parent->parent->value);
			if (strcmp(e->lexptr, "class") == 0) {
				fprintf(fd, "this.");
			}
		}
	}
	*/

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
		//print_echo(fd, command);
	}
	else
	if (strcmp(e->lexptr, "def") == 0) {
		print_definition(fd, command);
	}
	else
	if (strcmp(e->lexptr, "new") == 0) {
		//print_new(fd, command);
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
		//print_function(fd, command);
	}
	else
	if (strcmp(e->lexptr, "class") == 0) {
		print_class(fd, command);
	}
	else
	if (strcmp(e->lexptr, "return") == 0) {
		//print_return(fd, command);
	}
	else
	if (strcmp(e->lexptr, "array") == 0) {
		//print_array(fd, command);
	}
	else
	if (strcmp(e->lexptr, "if") == 0) {
		//print_if(fd, command);
	}
}
/*

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

*/
void print_class(FILE *fd, struct ast_node *command) {

	// get struct
	struct entry *ecmd = symtable_entryat(command->value);
	int structIndex = ecmd->value;
	const char *name = struct_table_get_name(structIndex);

	int previous_scope = scope;
	scope = structIndex;

	fprintf(fd, "struct %s {\n", name);

	int cchild = 1;

	// subclass
	struct ast_node *subclass = dd_da_get(&command->children, 1);
	struct entry *subentry = 0;
	if (subclass->node_type == AST_IDENTIFIER) {
		subentry = symtable_entryat(subclass->value);
		fprintf(fd, "struct %s parent;\n", subentry->lexptr);
		cchild++;
	}

	// definitions in struct
	struct ast_node *cmn = dd_da_get(&command->children, cchild);
	for (unsigned int i = 0; i < cmn->children.elements; i++) {
		struct ast_node *child = dd_da_get(&cmn->children, i);
		if (child->node_type == AST_COMMAND_NATIVE) {
			struct entry *e = symtable_entryat(child->value);
			if (strcmp(e->lexptr, "def") == 0) {
				print_definition(fd, child);
			}
			else
			if (strcmp(e->lexptr, "function") == 0) {
				struct ast_node *funcname = dd_da_get(&child->children, 0);
				struct entry *efuncname = symtable_entryat(funcname->value);

				// only include functions that are not overriding parent functions
				if (!struct_table_is_member_parent(structIndex, efuncname->lexptr)) {
					fprintf(fd, "void (*%s)(struct %s *", efuncname->lexptr, name);
					// function arguments
					fprintf(fd, ");\n");
				}
			}
		}
	}

	fprintf(fd, "};\n");

	// pre-define functions, so they are visible to all functions regardless of order
	for (unsigned int i = 0; i < cmn->children.elements; i++) {

		// grab ast node and symbol table entry, ensure this is a function
		struct ast_node *child = dd_da_get(&cmn->children, i);
		if (child->node_type != AST_COMMAND_NATIVE) continue;
		struct entry *echild = symtable_entryat(child->value);
		if (strcmp(echild->lexptr, "function") != 0) continue;

		// get function details
		struct ast_node *funcname = dd_da_get(&child->children, 0);
		struct entry *efuncname = symtable_entryat(funcname->value);

		// print the function signature
		fprintf(fd, "void %s_%s(struct %s *this", name, efuncname->lexptr, name);
		// function arguments
		fprintf(fd, ");\n");
	}

	// functions
	for (unsigned int i = 0; i < cmn->children.elements; i++) {

		// grab ast node and symbol table entry
		struct ast_node *child = dd_da_get(&cmn->children, i);
		if (child->node_type != AST_COMMAND_NATIVE) continue;
		struct entry *echild = symtable_entryat(child->value);

		// make sure to parse only functions at this point
		if (strcmp(echild->lexptr, "function") != 0) continue;

		// get function details
		struct ast_node *funcname = dd_da_get(&child->children, 0);
		struct entry *efuncname = symtable_entryat(funcname->value);

		// print the function signature
		fprintf(fd, "void %s_%s(struct %s *this", name, efuncname->lexptr, name);
		// function arguments go here
		fprintf(fd, ") {\n");

		// init function
		if (strcmp(efuncname->lexptr, "create") == 0) {
			// parent init
			if (subentry) {
				fprintf(fd, "%s_create(this);\n", subentry->lexptr);
			}

			// if any function of the class is the same as one of the parents, replace it
			for (unsigned int j = 0; j < cmn->children.elements; j++) {
				// grab ast node and symbol table entry
				struct ast_node *fchild = dd_da_get(&cmn->children, j);
				if (fchild->node_type != AST_COMMAND_NATIVE) continue;
				struct entry *efchild = symtable_entryat(fchild->value);
				if (strcmp(efchild->lexptr, "function") != 0) continue;

				struct ast_node *funcname2 = dd_da_get(&fchild->children, 0);
				struct entry *efuncname2 = symtable_entryat(funcname2->value);

				if (struct_table_is_member_parent(structIndex, efuncname2->lexptr)) {
					fprintf(fd, "this->parent.%s = %s_%s;\n", efuncname2->lexptr, name, efuncname2->lexptr);
				}
			}
		}

		int cmd_child = 2;
		struct ast_node *ovr = dd_da_get(&child->children, cmd_child);
		if (ovr->node_type == AST_IDENTIFIER) {
			struct entry *eovr = symtable_entryat(ovr->value);
			if (strcmp(eovr->lexptr, "override") == 0) {
				cmd_child++;
			}
		}
		print_node(fd, dd_da_get(&child->children, cmd_child));
		fprintf(fd, "}\n");
	}

	scope = previous_scope;

}
/*

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

*/
void print_identifier(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);

	if (command->children.elements > 0 && strcmp(e->lexptr, "this") == 0) {

		if (scope != -1) {
			struct ast_node *n = dd_da_get(&command->children, 0);
			struct entry *e = symtable_entryat(n->value);
			int memberId = struct_table_get_member(scope, e->lexptr);
			// if its not a primitive, add a & to access its memory
			if (!struct_table_is_member_primitive(scope, memberId)) {
				fprintf(fd, "&");
			}

		}
	}
	fprintf(fd, "%s", e->lexptr);

	for (unsigned int i = 0; i < command->children.elements; i++) {
		if (i == 0 && strcmp(e->lexptr, "this") == 0) {
			fprintf(fd, "->");
		}
		else {
			fprintf(fd, ".");
		}
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
			fprintf(fd, "\"%s\"", e->lexptr);
			break;
		}
		case AST_IDENTIFIER: {
			print_identifier(fd, n);
			break;
		}
		case AST_EMPTY: break;
	}
}

#ifndef INSTALL_LOCATION
#define INSTALL_LOCATION "/usr"
#endif

#ifndef PROJECT_NAME
#define PROJECT_NAME "ddlang"
#endif

// responsible for creating a file and translating ast to target language
void parse_cglut(const char *filename, struct ast_node *n) {
	dir_create("build-cglut");

	fd_global = fopen(filename, "w");
	if (!fd_global) {
		printf("unable to open '%s': %s\n", filename, strerror(errno));
		return;
	}
	fprintf(fd_global, "#include \"dd_filetomesh.h\"\n");
	fprintf(fd_global, "#include \"dd_world.h\"\n");
	fprintf(fd_global, "#include \"dd_object.h\"\n");
	fprintf(fd_global, "#include <stdio.h>\n");
	fprintf(fd_global, "#include <GL/glew.h>\n");
	print_node(fd_global, n);
	fprintf(fd_global, "void dd_world_init() {"
			"dd_world_change(sizeof(struct dd_world_main), (void (*)(struct dd_world *)) dd_world_main_create);"
		"}\n"
	);
	fclose(fd_global);

	int dir = open("build", O_DIRECTORY);
	if (!dir) {
		printf("error opening `build/`: %s\n", strerror(errno));
	}
	//sprintf(buffer, "%s/share/%s/dd_pixi_engine.js", INSTALL_LOCATION, PROJECT_NAME);
	//file_copy_at(0, buffer, dir, "dd_pixi_engine.js", 0);
	//sprintf(buffer, "%s/share/%s/index.html", INSTALL_LOCATION, PROJECT_NAME);
	//file_copy_at(0, buffer, dir, "index.html", 0);
	//sprintf(buffer, "%s/share/%s/pixi.min.js", INSTALL_LOCATION, PROJECT_NAME);
	//file_copy_at(0, buffer, dir, "pixi.min.js", 0);

	//dir_create("build/images");
	//dir_copy_recursive(0, "images", dir, "images");
	//close(dir);
}
