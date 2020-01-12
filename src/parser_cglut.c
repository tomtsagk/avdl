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
static void print_class_definition(FILE *fd, struct ast_node *command);
static void print_number(FILE *fd, struct ast_node *command);
static void print_float(FILE *fd, struct ast_node *command);
static void print_identifier_chain(FILE *fd, struct ast_node *command, int ignore_last);
static void print_identifier(FILE *fd, struct ast_node *command);
static void print_definition(FILE *fd, struct ast_node *command);
static void print_operator_binary(FILE *fd, struct ast_node *command);
static void print_function_call(FILE *fd, struct ast_node *command);
static void print_for(FILE *fd, struct ast_node *command);
static void print_if(FILE *fd, struct ast_node *command);
static void print_function_arguments(FILE *fd, struct ast_node *command);
static void print_echo(FILE *fd, struct ast_node *command);
static void print_function(FILE *fd, struct ast_node *command);
/*
static void print_return(FILE *fd, struct ast_node *command);
static void print_array(FILE *fd, struct ast_node *command);
static void print_new(FILE *fd, struct ast_node *command);
*/

static void print_if(FILE *fd, struct ast_node *command) {

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
/*

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
	struct ast_node *funcname = dd_da_get(&command->children, 0);
	struct entry *efuncname = symtable_entryat(funcname->value);
	print_identifier_chain(fd, funcname, 0);
	fprintf(fd, "(");

	int has_arg = 0;
	if (funcname->children.elements > 0) {
		print_identifier_chain(fd, funcname, 1);
		has_arg = 1;
	}

	int prev_semi = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 1; i < command->children.elements; i++) {
		if (i != 1 || has_arg) {
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
	struct entry *type_entry = symtable_entryat(vartype->value);
	char *type;
	int arrayElements = vartype->arraySize;
	if (vartype->node_type == AST_IDENTIFIER) {
		if (strcmp(type_entry->lexptr, "float") != 0
		&&  strcmp(type_entry->lexptr, "int") != 0) {
			fprintf(fd, "struct ");
		}
		type = type_entry->lexptr;
	}
	fprintf(fd, "%s ", type);

	struct ast_node *varname = dd_da_get(&command->children, 1);
	struct entry *evarname = symtable_entryat(varname->value);

	if (vartype->isRef) {
		evarname->isRef = 1;
		fprintf(fd, "*");
	}

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
	if (arrayElements > 0) {
		fprintf(fd, "[%d]", arrayElements);
	}

	if (command->children.elements >= 3) {
		fprintf(fd, " = ");
		print_node(fd, dd_da_get(&command->children, 2));
	}
	if (has_semicolon) {
		fprintf(fd, ";\n");
	}
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
		//print_new(fd, command);
	}
	else
	if (strcmp(e->lexptr, "=") == 0) {
		print_operator_binary(fd, command);
		if (has_semicolon) {
			fprintf(fd, ";\n");
		}
	}
	else
	if (strcmp(e->lexptr, "+") == 0
	||  strcmp(e->lexptr, "-") == 0
	||  strcmp(e->lexptr, "*") == 0
	||  strcmp(e->lexptr, "/") == 0
	||  strcmp(e->lexptr, "%") == 0
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
		//print_return(fd, command);
	}
	else
	if (strcmp(e->lexptr, "array") == 0) {
		//print_array(fd, command);
	}
	else
	if (strcmp(e->lexptr, "if") == 0) {
		print_if(fd, command);
	}
	else
	if (strcmp(e->lexptr, "for") == 0) {
		print_for(fd, command);
	}
}

void print_echo(FILE *fd, struct ast_node *command) {
	fprintf(fd, "printf(\"%%s\\n\", ");
	int temp_semicolon = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 0; i < command->children.elements; i++) {
		if (i > 0) {
			fprintf(fd, " ");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	has_semicolon = temp_semicolon;
	fprintf(fd, ");\n");
}

static void print_function(FILE *fd, struct ast_node *command) {

	fprintf(fd, "void ");

	//name
	struct ast_node *name = dd_da_get(&command->children, 0);
	print_node(fd, name);

	fprintf(fd, "(");

	//arguments
	/*
	has_semicolon = 0;
	struct ast_node *arg = dd_da_get(&command->children, 1);
	print_node(fd, arg);
	has_semicolon = 1;
	*/

	fprintf(fd, ") {\n");

	//commands
	struct ast_node *cmd = dd_da_get(&command->children, 2);
	print_node(fd, cmd);

	fprintf(fd, "};\n");

}

static void print_function_arguments(FILE *fd, struct ast_node *command) {
	for (unsigned int i = 0; i+1 < command->children.elements; i += 2) {
		if (i != 0) {
			fprintf(fd, ", ");
		}
		print_node(fd, dd_da_get(&command->children, i));
		fprintf(fd, " ");
		print_node(fd, dd_da_get(&command->children, i+1));
	}
}

void print_class(FILE *fd, struct ast_node *command) {

	// get name
	struct ast_node *classname = dd_da_get(&command->children, 0);
	struct entry *eclassname = symtable_entryat(classname->value);

	// get struct
	int structIndex = struct_table_get_index(eclassname->lexptr);
	const char *name = struct_table_get_name(structIndex);

	int previous_scope = scope;
	scope = structIndex;

	int cchild = 1;


	// subclass
	struct ast_node *subclass = dd_da_get(&command->children, 1);
	struct entry *subentry = 0;
	if (subclass->node_type == AST_IDENTIFIER) {
		subentry = symtable_entryat(subclass->value);
		cchild++;
	}

	// definitions in struct
	struct ast_node *cmn = dd_da_get(&command->children, cchild);

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
		struct ast_node *funcargs = dd_da_get(&child->children, 1);
		if (funcargs->children.elements >= 2) {
			fprintf(fd, ", ");
		}
		print_function_arguments(fd, funcargs);
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

				// initialise all non-primitive members
				if (strcmp(efchild->lexptr, "def") == 0 && fchild->children.elements > 1) {
					struct ast_node *defchild = dd_da_get(&fchild->children, 0);
					if (defchild->node_type == AST_IDENTIFIER) {
						struct entry *edefchild = symtable_entryat(defchild->value);
						if (strcmp(edefchild->lexptr, "int") != 0
						&&  strcmp(edefchild->lexptr, "float") != 0
						&&  strcmp(edefchild->lexptr, "string") != 0) {
							struct ast_node *varname = dd_da_get(&fchild->children, 1);
							struct entry *evarname = symtable_entryat(varname->value);
							fprintf(fd, "%s_create(&this->%s);\n", edefchild->lexptr, evarname->lexptr);
						}
					}
					continue;
				}
				else if (strcmp(efchild->lexptr, "function") != 0) continue;

				struct ast_node *funcname2 = dd_da_get(&fchild->children, 0);
				struct entry *efuncname2 = symtable_entryat(funcname2->value);

				int parent_level = struct_table_is_member_parent(structIndex, efuncname2->lexptr);
				if (parent_level > 0) {
					fprintf(fd, "this->");
					for (int i = 0; i < parent_level; i++) {
						fprintf(fd, "parent.");
					}
					fprintf(fd, "%s = %s_%s;\n", efuncname2->lexptr, name, efuncname2->lexptr);
				}
				else {
					fprintf(fd, "this->%s = %s_%s;\n", efuncname2->lexptr, name, efuncname2->lexptr);
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

static void print_class_definition(FILE *fd, struct ast_node *command) {

	// get name
	struct ast_node *classname = dd_da_get(&command->children, 0);
	struct entry *eclassname = symtable_entryat(classname->value);

	// get struct
	int structIndex = struct_table_get_index(eclassname->lexptr);
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
				if (struct_table_is_member_parent(structIndex, efuncname->lexptr) == 0) {
					fprintf(fd, "void (*%s)(struct %s *", efuncname->lexptr, name);
					// function arguments
					struct ast_node *funcargs = dd_da_get(&child->children, 1);
					if (funcargs->children.elements >= 2) {
						fprintf(fd, ", ");
					}
					print_function_arguments(fd, funcargs);
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
		struct ast_node *funcargs = dd_da_get(&child->children, 1);
		if (funcargs->children.elements >= 2) {
			fprintf(fd, ", ");
		}
		print_function_arguments(fd, funcargs);
		fprintf(fd, ");\n");
	}
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
void print_identifier_chain(FILE *fd, struct ast_node *command, int ignore_last) {
	struct entry *e = symtable_entryat(command->value);

	if (command->children.elements > 0) {
		int current_scope;
		if (strcmp(e->lexptr, "this") == 0) {
			current_scope = scope;
		}
		else {
			current_scope = struct_table_get_index(e->scope);
		}
		int current_child = 0;

		int target = command->children.elements -(ignore_last ? 1 : 0);
		while (current_child < target) {
			struct ast_node *n = dd_da_get(&command->children, current_child);
			struct entry *e = symtable_entryat(n->value);

			int memberId = struct_table_get_member(current_scope, e->lexptr);
			// not last child, update scope
			if (current_child < target-1) {
				if (!struct_table_is_member_primitive_string(current_scope, e->lexptr)) {
					current_scope = struct_table_get_member_scope_string(current_scope, e->lexptr);
				}
			}
			// last child, decide if it pointer or not
			else {
				if (!struct_table_is_member_primitive_string(current_scope, e->lexptr) && !e->isRef) {
					fprintf(fd, "&");
				}
			}

			current_child++;
		}
	}
	print_identifier(fd, command);

	int lscope = -1;
	int nextRef = e->isRef;
	// print children
	for (unsigned int i = 0; i < command->children.elements -(ignore_last ? 1 : 0); i++) {
		struct ast_node *child = dd_da_get(&command->children, i);
		if (child->node_type == AST_GROUP && i == 0) continue;
		struct entry *echild = symtable_entryat(child->value);

		if ((i == 0 && strcmp(e->lexptr, "this") == 0) || nextRef) {
			fprintf(fd, "->");
			if (!nextRef) {
				lscope = scope;
			}
			else
			{
				lscope = struct_table_get_index(e->scope);
			}
			nextRef = 0;
		}
		else {
			if (i == 0) {
				lscope = -1;
			}
			fprintf(fd, ".");
		}

		// if scope exists, check if the following identifier is part of a parent class
		if (lscope >= 0) {
			int parent_level = struct_table_is_member_parent(lscope, echild->lexptr);
			if (parent_level >= 0) {
				for (int j = 0; j < parent_level; j++) {
					fprintf(fd, "parent.");
				}
			}
		}

		print_identifier(fd, child);
		if (echild->isRef) {
			nextRef = 1;
		}

		// update scope
		if (lscope >= 0 && !struct_table_is_member_primitive(lscope, struct_table_get_member(lscope, echild->lexptr))) {
			lscope = struct_table_get_member_scope(lscope, struct_table_get_member(lscope, echild->lexptr));
		}
	}
}

void print_identifier(FILE *fd, struct ast_node *command) {
	struct entry *e = symtable_entryat(command->value);
	fprintf(fd, "%s", e->lexptr);

	// check if member of array
	int cchild = 0;
	if (command->children.elements > 0) {
		struct ast_node *c = dd_da_get(&command->children, cchild);
		if (c->node_type == AST_GROUP) {
			cchild++;
			fprintf(fd, "[");
			print_node(fd, c);
			fprintf(fd, "]");
		}
	}
}

static void print_for(FILE *fd, struct ast_node *command) {
	fprintf(fd, "for (");
	int prev_semicolon = has_semicolon;
	has_semicolon = 0;
	print_node(fd, dd_da_get(&command->children, 0));
	fprintf(fd, ";");
	print_node(fd, dd_da_get(&command->children, 1));
	fprintf(fd, ";");
	print_node(fd, dd_da_get(&command->children, 2));
	has_semicolon = prev_semicolon;
	fprintf(fd, ") {\n");
	print_node(fd, dd_da_get(&command->children, 3));
	fprintf(fd, "}\n");
}

void print_node(FILE *fd, struct ast_node *n) {
	switch (n->node_type) {
		case AST_GAME:
		case AST_GROUP:
			// for classes, pre-define them so they can interact with each other
			for (unsigned int i = 0; i < n->children.elements; i++) {
				struct ast_node *child = dd_da_get(&n->children, i);
				if (child->node_type == AST_COMMAND_NATIVE) {
					struct entry *e = symtable_entryat(child->value);
					if (strcmp(e->lexptr, "class") == 0) {
						print_class_definition(fd, child);
					}
				}
			}
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
			print_identifier_chain(fd, n, 0);
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

	if (n) {
		dir_create("build-cglut");

		fd_global = fopen(filename, "w");
		if (!fd_global) {
			printf("unable to open '%s': %s\n", filename, strerror(errno));
			return;
		}
		fprintf(fd_global, "#include \"dd_ddcglut.h\"\n");
		fprintf(fd_global, "#include <stdio.h>\n");
		fprintf(fd_global, "#include <GL/glew.h>\n");
		fprintf(fd_global, "int main(int argc, char *argv[]) {dd_main(argc, argv);}\n");
		print_node(fd_global, n);
		fprintf(fd_global, "void dd_world_init() {"
				"dd_world_set(dd_world_main);"
			"}\n"
		);
		fclose(fd_global);
	}

	system("gcc build-cglut/game.c -O3 -o build-cglut/game -lGL -lGLU -lGLEW -lglut -lddcglut -lm -w -lSDL2 -lSDL2_mixer");

	/*
	int dir = open("build", O_DIRECTORY);
	if (!dir) {
		printf("error opening `build/`: %s\n", strerror(errno));
	}
	*/

	//dir_create("build/images");
	//dir_copy_recursive(0, "images", dir, "images");
	//close(dir);
}
