#include "parser.h"
#include "stdlib.h"
#include "stdio.h"
#include "symtable.h"
#include "struct_table.h"
#include <string.h>
#include "file_op.h"
#include <errno.h>
#include "agc_commands.h"

static void print_command_definition(FILE *fd, struct ast_node *n);
static void print_command_definitionClassFunction(FILE *fd, struct ast_node *n, const char *classname);
static void print_command_functionArguments(FILE *fd, struct ast_node *n, int beginWithSemicolon);
static void print_command_class(FILE *fd, struct ast_node *n);
static void print_command_native(FILE *fd, struct ast_node *n);
static void print_command_classFunction(FILE *fd, struct ast_node *n);
static void print_command_function(FILE *fd, struct ast_node *n);
static void print_command_custom(FILE *fd, struct ast_node *n);
static void print_command_echo(FILE *fd, struct ast_node *n);
static void print_command_if(FILE *fd, struct ast_node *n);
static void print_command_for(FILE *fd, struct ast_node *n);
static void print_command_multistring(FILE *fd, struct ast_node *n);
static void print_command_return(FILE *fd, struct ast_node *n);
static void print_command_groupStatements(FILE *fd, struct ast_node *n);
static void print_binaryOperation(FILE *fd, struct ast_node *n);
static void print_identifierReference(FILE *fd, struct ast_node *n, int skipLast);
static void print_identifier(FILE *fd, struct ast_node *n, int skipLast);
static void print_number(FILE *fd, struct ast_node *n);
static void print_float(FILE *fd, struct ast_node *n);
static void print_node(FILE *fd, struct ast_node *n);
static int getIdentifierChainCount(struct ast_node *n);
static struct ast_node *getIdentifierLast(struct ast_node *n);
static struct ast_node *getIdentifierInChain(struct ast_node *n, int position);

static struct ast_node *getIdentifierInChain(struct ast_node *n, int position) {

	if (position == 0) {
		return n;
	}

	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);
		if (child->node_type == AST_IDENTIFIER) {
			return getIdentifierInChain(child, position-1);
		}
	}

	return n;
}

static void print_command_groupStatements(FILE *fd, struct ast_node *n) {

	if (n->node_type == AST_COMMAND_NATIVE && strcmp(n->lex, "group") == 0) {
		for (unsigned int i = 0; i < n->children.elements; i++) {
			print_node(fd, dd_da_get(&n->children, i));
			fprintf(fd, ";\n");
		}
	}
	else {
		print_node(fd, n);
		fprintf(fd, ";\n");
	}
}

static void print_command_return(FILE *fd, struct ast_node *n) {
	fprintf(fd, "return ");
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);
		print_node(fd, child);
	}
}

static void print_command_multistring(FILE *fd, struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *string = dd_da_get(&n->children, i);
		print_node(fd, string);
	}
}

static void print_command_for(FILE *fd, struct ast_node *n) {
	struct ast_node *definition = dd_da_get(&n->children, 0);
	struct ast_node *condition = dd_da_get(&n->children, 1);
	struct ast_node *step = dd_da_get(&n->children, 2);
	struct ast_node *statements = dd_da_get(&n->children, 3);

	fprintf(fd, "for (");
	print_node(fd, definition);
	print_node(fd, condition);
	fprintf(fd, ";");
	print_node(fd, step);
	fprintf(fd, ") {\n");
	print_command_groupStatements(fd, statements);
	fprintf(fd, "}");
}

static void print_command_if(FILE *fd, struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i += 2) {
		struct ast_node *child1 = dd_da_get(&n->children, i);
		struct ast_node *child2 = 0;

		if (i+1 < n->children.elements) {
			child2 = dd_da_get(&n->children, i+1);
		}

		if (i != 0) {
			fprintf(fd, "else\n");
		}

		if (child2) {
			fprintf(fd, "if (");
			print_node(fd, child1);
			fprintf(fd, ") {\n");
			print_command_groupStatements(fd, child2);
			fprintf(fd, "}\n");
		}
		else {
			fprintf(fd, "{\n");
			print_command_groupStatements(fd, child1);
			fprintf(fd, "}\n");
		}
	}
}

static void print_command_echo(FILE *fd, struct ast_node *n) {

	fprintf(fd, "printf(\"");
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);

		if (child->node_type == AST_STRING) {
			fprintf(fd, "%%s");
		}
		else
		if (child->node_type == AST_NUMBER) {
			fprintf(fd, "%%d");
		}
		else
		if (child->node_type == AST_FLOAT) {
			fprintf(fd, "%%f");
		}
		else
		if (child->node_type == AST_IDENTIFIER
		&&  child->value == DD_VARIABLE_TYPE_INT) {
			fprintf(fd, "%%d");
		}
		else
		if (child->node_type == AST_IDENTIFIER
		&&  child->value == DD_VARIABLE_TYPE_FLOAT) {
			fprintf(fd, "%%f");
		}
		else {
			printf("unable to parse argument in echo '%s(%d)', no rule to parse it (1)\n",
				child->lex, child->value);
			exit(-1);
		}
	}
	fprintf(fd, "\\n\"");
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);

		fprintf(fd, ", ");
		if (child->node_type == AST_STRING) {
			fprintf(fd, "\"%s\"", child->lex);
		}
		else
		if (child->node_type == AST_NUMBER) {
			fprintf(fd, "%d", child->value);
		}
		else
		if (child->node_type == AST_FLOAT) {
			fprintf(fd, "%f", child->fvalue);
		}
		else
		if (child->node_type == AST_IDENTIFIER
		&&  child->value == DD_VARIABLE_TYPE_INT) {
			fputs(child->lex, fd);
		}
		else
		if (child->node_type == AST_IDENTIFIER
		&&  child->value == DD_VARIABLE_TYPE_FLOAT) {
			fputs(child->lex, fd);
		}
		else {
			printf("unable to parse argument in echo '%s(%d)', no rule to parse it (2)\n",
				child->lex, child->value);
			exit(-1);
		}
	}
	fprintf(fd, ");\n");
}

static void print_binaryOperation(FILE *fd, struct ast_node *n) {
	struct ast_node *child1 = dd_da_get(&n->children, 0);

	if (strcmp(n->lex, "=") != 0) {
		fprintf(fd, "(");
	}

	if (strcmp(n->lex, "=") == 0) {
		print_identifier(fd, child1, 0);
	}
	else {
		print_node(fd, child1);
	}
	fprintf(fd, " ");

	for (int i = 1; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);
		fprintf(fd, "%s ", n->lex);
		print_node(fd, child);
	}

	if (strcmp(n->lex, "=") != 0) {
		fprintf(fd, ")");
	}
}

static void print_command_custom(FILE *fd, struct ast_node *n) {
	struct ast_node *cmdname = dd_da_get(&n->children, 0);

	print_identifier(fd, cmdname, 0);
	fprintf(fd, "(");
	int hasArgs = 0;
	if (strcmp(cmdname->lex, "this") == 0) {

		int chainCount = getIdentifierChainCount(cmdname);
		struct ast_node *semilast = getIdentifierInChain(cmdname, chainCount-1);

		// not dereferencing "this" hack
		if (chainCount-1 > 0 && !semilast->isRef) {
			fprintf(fd, "&");
		}

		print_identifier(fd, cmdname, 1);
		hasArgs = 1;
	}
	for (int i = 1; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);

		if (hasArgs) {
			fprintf(fd, ", ");
		}
		hasArgs = 1;

		print_node(fd, child);
	}
	fprintf(fd, ")");
}

static void print_command_function(FILE *fd, struct ast_node *n) {
	struct ast_node *functype = dd_da_get(&n->children, 0);
	struct ast_node *funcname = dd_da_get(&n->children, 1);
	struct ast_node *funcargs = dd_da_get(&n->children, 2);
	struct ast_node *funcstatements = 0;

	if (n->children.elements == 4) {
		funcstatements = dd_da_get(&n->children, 3);
	}

	// print function signature and args
	fprintf(fd, "%s %s(", functype->lex, funcname->lex);
	print_command_functionArguments(fd, funcargs, 1);
	fprintf(fd, ") {\n");

	if (funcstatements) {
		print_command_groupStatements(fd, funcstatements);
	}

	fprintf(fd, "}\n");

	// `dd_gameInit` is the "main" function, where everything begins
	if (strcmp(funcname->lex, "dd_gameInit") == 0) {
		fprintf(fd, "int main(int argc, char *argv[]) {dd_main(argc, argv);}\n");
	}
}

static void print_command_classFunction(FILE *fd, struct ast_node *n) {
	struct ast_node *classname = dd_da_get(&n->children, 0);
	struct ast_node *function = dd_da_get(&n->children, 1);

	struct ast_node *functype = dd_da_get(&function->children, 0);
	struct ast_node *funcname = dd_da_get(&function->children, 1);
	struct ast_node *funcargs = dd_da_get(&function->children, 2);
	struct ast_node *funcstatements = dd_da_get(&function->children, 3);

	int structIndex = struct_table_get_index(classname->lex);

	// print function signature and args
	fprintf(fd, "%s %s_%s(struct %s *this", functype->lex, classname->lex,
		funcname->lex, classname->lex);
	print_command_functionArguments(fd, funcargs, 1);
	fprintf(fd, ") {\n");

	// create functions are special
	if (strcmp(funcname->lex, "create") == 0) {

		// subclass init
		int subclassIndex = struct_table_get_parent(structIndex);
		if (subclassIndex >= 0) {
			fprintf(fd, "%s_%s(this);\n", struct_table_get_name(subclassIndex),
				funcname->lex);
		}

		// prepare functions
		for (int i = 0; i < struct_table_get_member_total(structIndex); i++) {
			if (struct_table_get_member_type(structIndex, i) == DD_VARIABLE_TYPE_FUNCTION) {
				const char *memberFuncname = struct_table_get_member_name(structIndex, i);
				int parentDepth = struct_table_is_member_parent(structIndex, memberFuncname);
				fprintf(fd, "this->");
				for (int j = 0; j < parentDepth; j++) {
					fprintf(fd, "parent.");
				}
				fprintf(fd, "%s = %s_%s;\n", memberFuncname, classname->lex, memberFuncname);
			}
		}

		// initialise structs
		for (int i = 0; i < struct_table_get_member_total(structIndex); i++) {
			if (struct_table_get_member_type(structIndex, i) == DD_VARIABLE_TYPE_STRUCT) {
				const char *memberName = struct_table_get_member_name(structIndex, i);
				char *memberType = struct_table_get_member_nametype(structIndex, i);
				int arrayCount = struct_table_getMemberArrayCount(structIndex, i);
				int isRef = struct_table_getMemberIsRef(structIndex, i);
				if (isRef) continue;
				if (arrayCount > 1) {
					fprintf(fd, "for (int i = 0; i < %d; i++) {\n", arrayCount);
					fprintf(fd, "	%s_create(&this->%s[i]);\n", memberType, memberName);
					fprintf(fd, "}\n");
				}
				else {
					fprintf(fd, "%s_create(&this->%s);\n", memberType, memberName);
				}
				//int parentDepth = struct_table_is_member_parent(structIndex, memberName);
			}
		}
	}
	else
	// `clean` function cleans all structs
	if (strcmp(funcname->lex, "clean") == 0) {
		// initialise structs
		for (int i = 0; i < struct_table_get_member_total(structIndex); i++) {
			if (struct_table_get_member_type(structIndex, i) == DD_VARIABLE_TYPE_STRUCT) {
				const char *memberName = struct_table_get_member_name(structIndex, i);
				char *memberType = struct_table_get_member_nametype(structIndex, i);
				//int parentDepth = struct_table_is_member_parent(structIndex, memberName);
				int isRef = struct_table_getMemberIsRef(structIndex, i);
				if (isRef) continue;
				fprintf(fd, "%s_clean(&this->%s);\n", memberType, memberName);
			}
		}
	}

	// print function statements
	print_command_groupStatements(fd, funcstatements);

	fprintf(fd, "}\n");
}

static int getIdentifierChainCount(struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);
		if (child->node_type == AST_IDENTIFIER) {
			return getIdentifierChainCount(child) +1;
		}
	}

	return 0;
}

static struct ast_node *getIdentifierLast(struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);
		if (child->node_type == AST_IDENTIFIER) {
			return getIdentifierLast(child);
		}
	}

	return n;
}

static void print_identifierReference(FILE *fd, struct ast_node *n, int skipLast) {

	// check if reference is needed
	struct ast_node *last = getIdentifierLast(n);
	if (last->value == DD_VARIABLE_TYPE_STRUCT
	&&  !last->isRef) {
		fprintf(fd, "&");
	}

	// print identifier
	print_identifier(fd, n, skipLast);
}

static void print_identifier(FILE *fd, struct ast_node *n, int skipLast) {

	fprintf(fd, "%s", n->lex);

	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = dd_da_get(&n->children, i);

		// has array
		if (child->node_type == AST_GROUP) {
			fprintf(fd, "[");
			print_node(fd, child);
			fprintf(fd, "]");
		}

		if (getIdentifierChainCount(n) == 1 && skipLast) {
			break;
		}

		if (child->node_type == AST_IDENTIFIER) {

			if (n->isRef) {
				fprintf(fd, "->");
			}
			else {
				fprintf(fd, ".");
			}
			print_identifier(fd, child, skipLast);
		}
	}
}

static void print_command_functionArguments(FILE *fd, struct ast_node *n, int beginWithSemicolon) {
	for (int i = 1; i < n->children.elements; i += 2) {
		struct ast_node *argtype = dd_da_get(&n->children, i-1);
		struct ast_node *argname = dd_da_get(&n->children, i  );

		if (i > 1 || beginWithSemicolon) {
			fprintf(fd, ", ");
		}

		if (!dd_variable_type_isPrimitiveType(argtype->lex)) {
			fprintf(fd, "struct ");
		}
		fprintf(fd, "%s ", argtype->lex);
		if (!dd_variable_type_isPrimitiveType(argtype->lex)) {
			fprintf(fd, "*");
		}
		fprintf(fd, "%s", argname->lex);
	}
}

static void print_command_definitionClassFunction(FILE *fd, struct ast_node *n, const char *classname) {
	struct ast_node *type = dd_da_get(&n->children, 0);
	struct ast_node *name = dd_da_get(&n->children, 1);
	struct ast_node *args = dd_da_get(&n->children, 2);
	fprintf(fd, "%s (*%s)(struct %s *", type->lex, name->lex, classname);
	print_command_functionArguments(fd, args, 1);
	fprintf(fd, ");\n");
}

static void print_command_definition(FILE *fd, struct ast_node *n) {
	struct ast_node *type = dd_da_get(&n->children, 0);
	struct ast_node *defname = dd_da_get(&n->children, 1);

	if (n->isExtern) {
		fprintf(fd, "extern ");
	}

	if (!dd_variable_type_isPrimitiveType(type->lex)) {
		fprintf(fd, "struct ");
	}
	fprintf(fd, "%s ", type->lex);
	if (n->isRef) {
		fprintf(fd, "*");
	}
	print_identifier(fd, defname, 0);

	if (n->children.elements >= 3) {
		struct ast_node *initValue = dd_da_get(&n->children, 2);
		fprintf(fd, " = ");
		print_node(fd, initValue);
	}
	fprintf(fd, ";\n");
}

static void print_command_class(FILE *fd, struct ast_node *n) {
	struct ast_node *classname = dd_da_get(&n->children, 0);
	struct ast_node *subclassname = dd_da_get(&n->children, 1);
	struct ast_node *definitions = dd_da_get(&n->children, 2);

	fprintf(fd, "struct %s {\n", classname->lex);

	// subclass
	if (subclassname->node_type == AST_IDENTIFIER) {
		fprintf(fd, "struct %s parent;\n", subclassname->lex);
	}

	// definitions in struct
	for (unsigned int i = 0; i < definitions->children.elements; i++) {
		struct ast_node *child = dd_da_get(&definitions->children, i);

		// definition of variable
		if (strcmp(child->lex, "def") == 0) {
			print_command_definition(fd, child);
		}
		// definition of function
		else {
			// function does not override another
			if (child->value == 0) {
				print_command_definitionClassFunction(fd, child, classname->lex);
			}
		}
	}

	fprintf(fd, "};\n");

	// pre-define functions, so they are visible to all functions regardless of order
	for (unsigned int i = 1; i < definitions->children.elements; i++) {

		// grab ast node and symbol table entry, ensure this is a function
		struct ast_node *child = dd_da_get(&definitions->children, i);
		if (child->node_type != AST_COMMAND_NATIVE
		||  strcmp(child->lex, "function") != 0) continue;

		// function name
		struct ast_node *functype = dd_da_get(&child->children, 0);
		struct ast_node *funcname = dd_da_get(&child->children, 1);

		// print the function signature
		fprintf(fd, "%s %s_%s(struct %s *this", functype->lex, classname->lex, funcname->lex, classname->lex);
		struct ast_node *args = dd_da_get(&child->children, 2);
		print_command_functionArguments(fd, args, 1);
		fprintf(fd, ");\n");
	}

} // print class definition

static void print_command_native(FILE *fd, struct ast_node *n) {
	if (strcmp(n->lex, "class") == 0) {
		print_command_class(fd, n);
	}
	else
	if (strcmp(n->lex, "class_function") == 0) {
		print_command_classFunction(fd, n);
	}
	else
	if (strcmp(n->lex, "function") == 0) {
		print_command_function(fd, n);
	}
	else
	if (strcmp(n->lex, "group") == 0) {
		for (unsigned int i = 0; i < n->children.elements; i++) {
			print_node(fd, dd_da_get(&n->children, i));
		}
	}
	else
	if (strcmp(n->lex, "def") == 0) {
		print_command_definition(fd, n);
	}
	else
	if (strcmp(n->lex, "=") == 0
	||  strcmp(n->lex, "+") == 0
	||  strcmp(n->lex, "-") == 0
	||  strcmp(n->lex, "*") == 0
	||  strcmp(n->lex, "/") == 0
	||  strcmp(n->lex, "%") == 0
	||  strcmp(n->lex, "&&") == 0
	||  strcmp(n->lex, "||") == 0
	||  strcmp(n->lex, "==") == 0
	||  strcmp(n->lex, ">=") == 0
	||  strcmp(n->lex, "<=") == 0
	||  strcmp(n->lex, ">") == 0
	||  strcmp(n->lex, "<") == 0) {
		print_binaryOperation(fd, n);
	}
	else
	if (strcmp(n->lex, "echo") == 0) {
		print_command_echo(fd, n);
	}
	else
	if (strcmp(n->lex, "if") == 0) {
		print_command_if(fd, n);
	}
	else
	if (strcmp(n->lex, "for") == 0) {
		print_command_for(fd, n);
	}
	else
	if (strcmp(n->lex, "multistring") == 0) {
		print_command_multistring(fd, n);
	}
	else
	if (strcmp(n->lex, "return") == 0) {
		print_command_return(fd, n);
	}
	else {
		printf("unable to parse command '%s': no parsing rule\n", n->lex);
		exit(-1);
	}
}

static void print_number(FILE *fd, struct ast_node *n) {
	fprintf(fd, "%d", n->value);
}

static void print_float(FILE *fd, struct ast_node *n) {
	fprintf(fd, "%f", n->fvalue);
}

static void print_node(FILE *fd, struct ast_node *n) {
	switch (n->node_type) {
		case AST_GAME:
		case AST_GROUP:
			for (unsigned int i = 0; i < n->children.elements; i++) {
				print_node(fd, dd_da_get(&n->children, i));
			}
			break;
		case AST_COMMAND_NATIVE: {
			print_command_native(fd, n);
			break;
		}
		case AST_COMMAND_CUSTOM: {
			print_command_custom(fd, n);
			break;
		}
		case AST_NUMBER: {
			print_number(fd, n);
			break;
		}
		case AST_IDENTIFIER: {
			print_identifierReference(fd, n, 0);
			break;
		}
		case AST_FLOAT: {
			print_float(fd, n);
			break;
		}
		case AST_STRING: {
			fprintf(fd, "\"%s\"", n->lex);
			break;
		}
		case AST_INCLUDE:
		case AST_EMPTY:
			break;
	}
}

// responsible for creating a file and translating ast to target language
int transpile_cglut(const char *filename, struct ast_node *n) {

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
	FILE *fd_global = fopen(filename, "w");
	if (!fd_global) {
		printf("avdl: transpile_cglut: unable to open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// print node to file
	fprintf(fd_global, "#include \"avdl_cengine.h\"\n");
	print_node(fd_global, n);

	// clean up
	if (fclose(fd_global) != 0) {
		printf("avdl: transpile_cglut: unable to close '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// everything OK
	return 0;

} // transpile_cglut
