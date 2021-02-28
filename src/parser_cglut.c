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
#include <time.h>
#include <errno.h>
#include "dd_commands.h"

#define DD_BUFFER_SIZE 1000
static char buffer[DD_BUFFER_SIZE];

// flags
static int has_semicolon = 1;

// file descriptor for global data
FILE *fd_global;

int scope = -1;

char asset_names[100][100];
int asset_names_index = -1;

static void print_node(FILE *fd, struct ast_node *n);

extern const char *primitive_keywords[];
extern unsigned int primitive_keywords_count;

/*
struct command_translation {
	char *command;
	char *translation;
};
*/

static void print_asset(FILE *fd, struct ast_node *command);
static void print_command(FILE *fd, struct ast_node *command);
//static void print_class(FILE *fd, struct ast_node *command);
static void print_class_definition(FILE *fd, struct ast_node *command);
static void print_class_function(FILE *fd, struct ast_node *command);
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
static void print_extern(FILE *fd, struct ast_node *command);
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
	//struct entry *efuncname = symtable_entryat(funcname->value);
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
		int isPrimitive = 0;
		for (unsigned int i = 0; i < primitive_keywords_count; i++) {
			if (strcmp(primitive_keywords[i], type_entry->lexptr) == 0) {
				isPrimitive = 1;
				break;
			}
		}
		if (!isPrimitive) {
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

void print_asset(FILE *fd, struct ast_node *command) {

	// get node
	struct ast_node *assetName = dd_da_get(&command->children, 0);
	struct entry *eassetName = symtable_entryat(assetName->value);

	/*
	 * for android, find the file name, without its extention
	 * and use that
	 */
	if (avdl_platform == AVDL_PLATFORM_ANDROID) {

		/*
		 * find the last '/' and last '.' on the file name,
		 * so if it has the form of `mydir/myfile.ply`
		 * it can be parsed to `myfile`
		 */
		char *lastSlash = 0;
		char *lastDot = 0;
		char *tempChar = eassetName->lexptr;
		while (tempChar[0] != '\0') {
			if (tempChar[0] == '/') {
				lastSlash = tempChar;
			}
			if (tempChar[0] == '.') {
				lastDot = tempChar;
			}
			tempChar++;
		}

		// no slash - start from the beginning of the file
		if (!lastSlash) {
			lastSlash = eassetName->lexptr;
		}
		// found slash - start from the next letter
		else {
			lastSlash++;
		}

		// no dot - parse until the end of the filename
		if (!lastDot) {
			lastDot = eassetName->lexptr +strlen(eassetName->lexptr);
		}

		// copy parsed string to buffer
		strncpy(buffer, lastSlash, lastDot -lastSlash);
		buffer[lastDot -lastSlash] = '\0';
	}
	else
	// on native, pass assets as-is
	if (avdl_platform == AVDL_PLATFORM_NATIVE) {

		strcpy(buffer, eassetName->lexptr);
		char *lastDot = 0;
		int currentChar = 0;
		while (buffer[currentChar] != '\0') {
			if (buffer[currentChar] == '.') {
				lastDot = buffer +currentChar;
			}
			currentChar++;
		}

		if (lastDot != 0) {
			lastDot[0] = '\0';
		}

		strcat(buffer, ".asset");
	}
	fprintf(fd, "\"%s\"", buffer);

	// print the final asset path (relative to the out_dir)
	//print_node(fd, assetName);

	/*
	// stat source and destination asset, if source is newer, copy to destination
	struct stat stat_src_asset;
	if (stat(eassetName->lexptr, &stat_src_asset) == -1) {
		printf("avdl error: Unable to stat '%s': %s\n", eassetName->lexptr, strerror(errno));
		exit(-1);
	}

	struct stat stat_dst_asset;
	strcpy(buffer, dirname);
	strcat(buffer, "/");
	strcat(buffer, eassetName->lexptr);
	if (stat(buffer, &stat_dst_asset) == -1) {
		//printf("error stat-ing destination: %s\n", buffer);
		file_copy_at(0, eassetName->lexptr, out_dir, eassetName->lexptr, 0);
	}

	// check last modification time
	time_t time_src = mktime(gmtime(&stat_src_asset.st_mtime));
	time_t time_dst = mktime(gmtime(&stat_dst_asset.st_mtime));

	if (time_src > time_dst) {
		file_copy_at(0, eassetName->lexptr, out_dir, eassetName->lexptr, 0);
	}
	*/
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
		print_class_definition(fd, command);
	}
	else
	if (strcmp(e->lexptr, "class_function") == 0) {
		print_class_function(fd, command);
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
	else
	if (strcmp(e->lexptr, "asset") == 0) {
		print_asset(fd, command);
	}
	else
	if (strcmp(e->lexptr, "extern") == 0) {
		print_extern(fd, command);
	}
}

void print_echo(FILE *fd, struct ast_node *command) {
	fprintf(fd, "dd_log(\"");

	// print the format depending on the children's types
	for (unsigned int i = 0; i < command->children.elements; i++) {
		struct ast_node *child = dd_da_get(&command->children, i);

		// Primitive types
		if (child->node_type == AST_NUMBER) {
			fprintf(fd, "%%d");
		}
		else
		if (child->node_type == AST_FLOAT) {
			fprintf(fd, "%%f");
		}
		else
		if (child->node_type == AST_STRING) {
			fprintf(fd, "%%s");
		}
		else
		// Identifier - check its type
		if (child->node_type == AST_IDENTIFIER) {
			struct entry *e = symtable_entryat(child->value);
			if (strcmp(e->scope, "int") == 0) {
				fprintf(fd, "%%d");
			}
			else
			if (strcmp(e->scope, "float") == 0) {
				fprintf(fd, "%%f");
			}
			else
			if (strcmp(e->scope, "string") == 0) {
				fprintf(fd, "%%s");
			}
		}
	}
	fprintf(fd, "\", ");

	// give the children to printf's format
	int temp_semicolon = has_semicolon;
	has_semicolon = 0;
	for (unsigned int i = 0; i < command->children.elements; i++) {
		if (i > 0) {
			fprintf(fd, ", ");
		}
		print_node(fd, dd_da_get(&command->children, i));
	}
	has_semicolon = temp_semicolon;
	fprintf(fd, ");\n");
}

static void print_function(FILE *fd, struct ast_node *command) {

	//name
	struct ast_node *name = dd_da_get(&command->children, 0);
	struct entry *ename = symtable_entryat(name->value);

	fprintf(fd, "void ");
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

	// `dd_gameInit` is the "main" function, where everything begins
	if (strcmp(ename->lexptr, "dd_gameInit") == 0) {
		fprintf(fd_global, "int main(int argc, char *argv[]) {dd_main(argc, argv);}\n");
	}
}

static void print_function_arguments(FILE *fd, struct ast_node *command) {
	for (unsigned int i = 0; i+1 < command->children.elements; i += 2) {
		if (i != 0) {
			fprintf(fd, ", ");
		}
		struct ast_node *c1 = dd_da_get(&command->children, i);
		struct entry *e1 = symtable_entryat(c1->value);
		int isPrimitive = 0;
		for (unsigned int i = 0; i < primitive_keywords_count; i++) {
			if (strcmp(primitive_keywords[i], e1->lexptr) == 0) {
				isPrimitive = 1;
				break;
			}
		}

		struct ast_node *type = dd_da_get(&command->children, i);
		struct ast_node *varname = dd_da_get(&command->children, i+1);
		if (!isPrimitive) {
			fprintf(fd, "struct ");
			//struct entry *etype = symtable_entryat(type->value);
			struct entry *evarname = symtable_entryat(varname->value);
			evarname->scope = e1->lexptr;
			evarname->isRef = 1;
		}

		print_node(fd, type);
		fprintf(fd, " ");
		if (!isPrimitive) {
			fprintf(fd, "*");
		}
		print_node(fd, varname);
	}
}

static void print_extern(FILE *fd, struct ast_node *command) {
	fprintf(fd, "extern ");
	for (unsigned int i = 0; i < command->children.elements; i++) {
		print_node(fd, dd_da_get(&command->children, i));
		fprintf(fd, " ");
	}
	fprintf(fd, ";\n");
}

/*
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

		print_node(fd, dd_da_get(&child->children, 2));
		//fprintf(fd, "}\n");
	}

	scope = previous_scope;

}
		*/

void print_class_function(FILE *fd, struct ast_node *command) {

	// get name
	struct ast_node *classname = dd_da_get(&command->children, 0);
	struct entry *eclassname = symtable_entryat(classname->value);

	int previous_scope = scope;
	scope = struct_table_get_index(eclassname->lexptr);

	// get function name
	struct ast_node *funcname = dd_da_get(&command->children, 1);
	struct entry *efuncname = symtable_entryat(funcname->value);

	// get struct
	int structIndex = struct_table_get_index(eclassname->lexptr);
	//const char *name = struct_table_get_name(structIndex);

	// subclass
	int subclassIndex = struct_table_get_parent(structIndex);
	/*
	struct ast_node *subclass = dd_da_get(&command->children, 1);
	struct entry *subentry = 0;
	if (subclass->node_type == AST_IDENTIFIER) {
		subentry = symtable_entryat(subclass->value);
	}
	*/

	struct ast_node *statements = dd_da_get(&command->children, 3);

	fprintf(fd, "void %s_%s(struct %s *this", eclassname->lexptr, efuncname->lexptr, eclassname->lexptr);
	// function arguments go here
	struct ast_node *funcargs = dd_da_get(&command->children, 2);
	if (funcargs->children.elements >= 2) {
		fprintf(fd, ", ");
	}
	print_function_arguments(fd, funcargs);
	//print_node(fd, dd_da_get(&command->children, 2));
	fprintf(fd, ") {\n");

	// function to create objects
	if (strcmp(efuncname->lexptr, "create") == 0) {

		// subclass
		if (subclassIndex >= 0) {
			fprintf(fd, "%s_create(this);\n", struct_table_get_name(subclassIndex));
		}

		// `create` function, add initialisation of functions
		//
		// if any function of the class is the same as one of the parents, replace it
		for (unsigned int j = 0; j < struct_table_get_member_total(structIndex); j++) {

			// call the constructor for all non-primitive members
			if (!struct_table_is_member_primitive(structIndex, j)) {

				// if an array, initialise all members
				int arrayCount = struct_table_getMemberArrayCount(structIndex, j);
				if (arrayCount > 1) {
					fprintf(fd, "for (int i = 0; i < %d; i++) {\n", arrayCount);
					fprintf(fd, "%s_create(&this->%s[i]);\n}\n",
						struct_table_get_member_nametype(structIndex, j),
						struct_table_get_member_name(structIndex, j)
					);
				}
				// no array, just initialise member
				else {
					fprintf(fd, "%s_create(&this->%s);\n",
						struct_table_get_member_nametype(structIndex, j),
						struct_table_get_member_name(structIndex, j)
					);
				}

			}
			else if (struct_table_get_member_type(structIndex, j) == DD_VARIABLE_TYPE_FUNCTION) {
				int parent_level =
					struct_table_is_member_parent(structIndex, struct_table_get_member_name(structIndex, j));
				fprintf(fd, "this->");
				for (int i = 0; i < parent_level; i++) {
					fprintf(fd, "parent.");
				}
				fprintf(fd, "%s = %s_%s;\n",
					struct_table_get_member_name(structIndex, j),
					struct_table_get_name(structIndex),
					struct_table_get_member_name(structIndex, j)
				);
			}
		}
	}
	// function to clean objects in class
	else if (strcmp(efuncname->lexptr, "clean") == 0) {

		// subclass
		if (subclassIndex >= 0) {
			fprintf(fd, "%s_clean(this);\n", struct_table_get_name(subclassIndex));
		}

		// for each member in struct
		for (unsigned int j = 0; j < struct_table_get_member_total(structIndex); j++) {

			// clean all non-primitive members in struct
			if (!struct_table_is_member_primitive(structIndex, j)) {

				// if an array, clean all members
				int arrayCount = struct_table_getMemberArrayCount(structIndex, j);
				if (arrayCount > 1) {
					fprintf(fd, "for (int i = 0; i < %d; i++) {\n", arrayCount);
					fprintf(fd, "%s_clean(&this->%s[i]);\n}\n",
						struct_table_get_member_nametype(structIndex, j),
						struct_table_get_member_name(structIndex, j)
					);
				}
				// no array, just clean member
				else {
					fprintf(fd, "%s_clean(&this->%s);\n",
						struct_table_get_member_nametype(structIndex, j),
						struct_table_get_member_name(structIndex, j)
					);
				}

			}

		} // for each member

	} // clean objects

	print_node(fd, statements);
	fprintf(fd, "}\n");

	scope = previous_scope;
}

static void print_class_definition(FILE *fd, struct ast_node *command) {

	// get name
	struct ast_node *classname = dd_da_get(&command->children, 0);
	struct entry *eclassname = symtable_entryat(classname->value);

	// get struct
	int structIndex = struct_table_get_index(eclassname->lexptr);
	const char *name = struct_table_get_name(structIndex);

	//int previous_scope = scope;
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

	// decide if identifier chain is for a primitive or a structure
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
		while (current_child < target && current_scope >= 0) {
			struct ast_node *n = dd_da_get(&command->children, current_child);
			struct entry *e = symtable_entryat(n->value);

			//int memberId = struct_table_get_member(current_scope, e->lexptr);
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
	if (n->isIncluded) {
		return;
	}

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
		case AST_INCLUDE: {
			struct entry *e = symtable_entryat(n->value);
			strcpy(buffer, e->lexptr);
			buffer[strlen(buffer)-3] = 'h';
			buffer[strlen(buffer)-2] = '\0';
			fprintf(fd, "#include \"%s\"\n", buffer);
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
#define PROJECT_NAME "avdl"
#endif

// responsible for creating a file and translating ast to target language
int transpile_cglut(const char *filename, struct ast_node *n, int isIncluded) {

	/*
	 * make sure given filename and node
	 * have a value
	 */
	if (!filename) {
		printf("avdl: transpile_cglut: no filename given to transpile to\n");
		return -1;
	}

	if (!n) {
		printf("avdl: transpile_cglut: no node given to transpile\n");
		return -1;
	}

	// open given file to write to
	fd_global = fopen(filename, "w");
	if (!fd_global) {
		printf("avdl: transpile_cglut: unable to open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// on header files, create header guards
	if (isIncluded) {
		strcpy(buffer, filename);
		for (int i = 0; buffer[i] != '\0'; i++) {
			if (!(buffer[i] >= 'a' && buffer[i] <= 'z')
			&&  !(buffer[i] >= 'A' && buffer[i] <= 'Z')
			&& buffer[i] != '_') {
				buffer[i] = '_';
			}
		}
		fprintf(fd_global, "#ifndef DD_%s_H\n", buffer);
		fprintf(fd_global, "#define DD_%s_H\n", buffer);
	}

	// print node to file
	fprintf(fd_global, "#include \"avdl_cengine.h\"\n");
	print_node(fd_global, n);

	// on header files, end header guard
	if (isIncluded) {
		fprintf(fd_global, "#endif\n");
	}

	// clean up
	if (fclose(fd_global) != 0) {
		printf("avdl: transpile_cglut: unable to close '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// everything OK
	return 0;

} // transpile_cglut
