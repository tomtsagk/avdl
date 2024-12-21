#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "avdl_symtable.h"
#include "avdl_struct_table.h"
#include "avdl_commands.h"
#include "avdl_semantic_analyser.h"
#include "avdl_lexer.h"
#include "avdl_pkg.h"
#include "avdl_log.h"
#include "avdl_settings.h"
#include "avdl_json.h"
#include "avdl_ast_node.h"

// TODO: Possibly remove this
enum AVDL_PLATFORM avdl_platform_temp;
extern struct AvdlSettings *avdl_settings_ptr;

extern const char *avdl_project_path;

static struct ast_node *expect_command_definition(struct avdl_lexer *l);
static struct ast_node *expect_command_definitionShort(struct avdl_lexer *l, struct ast_node *n);
static struct ast_node *expect_command_enum(struct avdl_lexer *l);
static struct ast_node *expect_command_struct(struct avdl_lexer *l);
static struct ast_node *expect_command_classDefinition(struct avdl_lexer *l);
static struct ast_node *expect_command_group(struct avdl_lexer *l);
static struct ast_node *expect_command_functionDefinition(struct avdl_lexer *l);
static struct ast_node *expect_command_classFunction(struct avdl_lexer *l);
static struct ast_node *expect_identifier(struct avdl_lexer *l);
static struct ast_node *expect_int(struct avdl_lexer *l);
static struct ast_node *expect_float(struct avdl_lexer *l);
static struct ast_node *expect_string(struct avdl_lexer *l);
static struct ast_node *expect_command_binaryOperation(struct avdl_lexer *l, const char *binaryOperationLex);
static struct ast_node *expect_command(struct avdl_lexer *l);
static struct ast_node *expect_command_arg(struct avdl_lexer *l);
static struct ast_node *expect_command_if(struct avdl_lexer *l);
static struct ast_node *expect_command_include(struct avdl_lexer *l);
static struct ast_node *expect_command_asset(struct avdl_lexer *l);
static struct ast_node *expect_command_for(struct avdl_lexer *l);
static struct ast_node *expect_command_break(struct avdl_lexer *l);
static struct ast_node *expect_command_continue(struct avdl_lexer *l);
static struct ast_node *expect_command_multistring(struct avdl_lexer *l);
static struct ast_node *expect_command_unicode(struct avdl_lexer *l);
static struct ast_node *expect_command_return(struct avdl_lexer *l);
static void semantic_error(struct avdl_lexer *l, const char *msg, ...);

static struct ast_node *expect_command_return(struct avdl_lexer *l) {
	struct ast_node *returncmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(returncmd, 0);
	ast_setLex(returncmd, "return");

	if (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
		ast_addChild(returncmd, expect_command_arg(l));
	}

	return returncmd;
}

static struct ast_node *expect_command_multistring(struct avdl_lexer *l) {
	struct ast_node *multistringcmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(multistringcmd, 0);
	ast_setLex(multistringcmd, "multistring");

	while (avdl_lexer_peek(l) == LEXER_TOKEN_STRING || avdl_lexer_peek(l) == LEXER_TOKEN_IDENTIFIER) {
		if (avdl_lexer_peek(l) == LEXER_TOKEN_STRING) {
			ast_addChild(multistringcmd, expect_string(l));
		}
		else
		if (avdl_lexer_peek(l) == LEXER_TOKEN_IDENTIFIER) {
			ast_addChild(multistringcmd, expect_identifier(l));
		}
	}

	return multistringcmd;
}

static struct ast_node *expect_command_unicode(struct avdl_lexer *l) {
	struct ast_node *unicodecmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(unicodecmd, 0);
	ast_setLex(unicodecmd, "unicode");

	ast_addChild(unicodecmd, expect_string(l));

	return unicodecmd;
}

static struct ast_node *expect_command_for(struct avdl_lexer *l) {
	struct ast_node *definition = expect_command(l);
	struct ast_node *condition = expect_command(l);
	struct ast_node *step = expect_command(l);
	struct ast_node *statements = expect_command(l);

	struct ast_node *forcmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(forcmd, 0);
	ast_setLex(forcmd, "for");

	ast_addChild(forcmd, definition);
	ast_addChild(forcmd, condition);
	ast_addChild(forcmd, step);
	ast_addChild(forcmd, statements);

	return forcmd;
}

static struct ast_node *expect_command_break(struct avdl_lexer *l) {
	struct ast_node *returncmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(returncmd, 0);
	ast_setLex(returncmd, "break");
	return returncmd;
}

static struct ast_node *expect_command_continue(struct avdl_lexer *l) {
	struct ast_node *returncmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(returncmd, 0);
	ast_setLex(returncmd, "continue");
	return returncmd;
}

extern char *installLocation;
extern int installLocationDynamic;
static struct ast_node *expect_command_asset(struct avdl_lexer *l) {
	struct ast_node *asset = ast_create(AST_ASSET);
	ast_addChild(asset, expect_string(l));
	struct ast_node *assetName = ast_getChild(asset, 0);

	/*
	 * on android, a path to a file is truncated to the filename
	 * minus the ending.
	 *
	 * temporary solution
	 */
	if (avdl_platform_temp == AVDL_PLATFORM_ANDROID
	||  avdl_platform_temp == AVDL_PLATFORM_QUEST2) {
		char buffer[500];
		strcpy(buffer, ast_getLex(assetName));

		if (strcmp(buffer +strlen(buffer) -4, ".ply") == 0
		||  strcmp(buffer +strlen(buffer) -5, ".json") == 0
		||  strcmp(buffer +strlen(buffer) -4, ".ttf") == 0
		||  strcmp(buffer +strlen(buffer) -4, ".glb") == 0) {
			char *lastSlash = buffer;
			char *p = buffer;
			while (p[0] != '\0') {
				if (p[0] == '/') {
					lastSlash = p+1;
				}
				p++;
			}

			ast_setLex(assetName, lastSlash);
		}
		else {

			char *lastSlash = buffer;
			char *p = buffer;
			while (p[0] != '\0') {
				if (p[0] == '/') {
					lastSlash = p+1;
				}
				p++;
			}

			char *lastDot = buffer;
			p = buffer;
			while (p[0] != '\0') {
				if (p[0] == '.') {
					lastDot = p;
				}
				p++;
			}
			lastDot[0] = '\0';

			ast_setLex(assetName, lastSlash);
		}
	}
	else
	if ( avdl_platform_temp == AVDL_PLATFORM_D3D11) {
		char buffer[500];
		strcpy(buffer, ast_getLex(assetName));

		int len = strlen(buffer);
		if (strcmp(buffer +len -4, ".png") == 0) {
			buffer[len -3] = 'd';
			buffer[len -2] = 'd';
			buffer[len -1] = 's';
			ast_setLex(assetName, buffer);
		}
	}
	else
	// on linux and windows, attach the custom install location as the asset's prefix
	if (avdl_platform_temp == AVDL_PLATFORM_LINUX
	||  avdl_platform_temp == AVDL_PLATFORM_WINDOWS) {
		/*
		if (!installLocationDynamic) {
			char buffer[500];
			strcpy(buffer, installLocation);
			strcat(buffer, ast_getLex(asset));
			ast_setLex(asset, buffer);
		}
		*/
	}

	// add the type as sibling - currently only identifiers count
	ast_addChild(asset, expect_identifier(l));

	return asset;
}

//extern char *saveLocation;
static struct ast_node *expect_command_savefile(struct avdl_lexer *l) {
	struct ast_node *savefile = expect_string(l);

	/*
	 * on android, a path to a file is truncated to the filename
	 * minus the ending.
	 *
	 * temporary solution
	 */
	if (avdl_platform_temp == AVDL_PLATFORM_ANDROID
	||  avdl_platform_temp == AVDL_PLATFORM_QUEST2) {
		/*
		char buffer[500];
		strcpy(buffer, ast_getLex(savefile));

		char *lastSlash = buffer;
		char *p = buffer;
		while (p[0] != '\0') {
			if (p[0] == '/') {
				lastSlash = p+1;
			}
			p++;
		}

		char *lastDot = buffer;
		p = buffer;
		while (p[0] != '\0') {
			if (p[0] == '.') {
				lastDot = p;
			}
			p++;
		}
		lastDot[0] = '\0';

		ast_setLex(savefile, lastSlash);
		*/
	}
	else
	// on linux and windows, attach the custom install location as the asset's prefix
	if (avdl_platform_temp == AVDL_PLATFORM_LINUX
	||  avdl_platform_temp == AVDL_PLATFORM_WINDOWS) {
		/*
		char buffer[500];
		if (saveLocation) {
			strcpy(buffer, saveLocation);
			strcat(buffer, ast_getLex(savefile));
			ast_setLex(savefile, buffer);
		}
		*/
	}

	return savefile;
}

static struct ast_node *expect_command_include(struct avdl_lexer *l) {
	struct ast_node *include = ast_create(AST_INCLUDE);
	ast_setValuei(include, 0);
	struct ast_node *filename = expect_string(l);
	ast_setLex(include, ast_getLex(filename));
	ast_delete(filename);
	return include;
}

static struct ast_node *expect_command_if(struct avdl_lexer *l) {

	struct ast_node *ifcmd = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(ifcmd, 0);
	ast_setLex(ifcmd, "if");

	while (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
		ast_addChild(ifcmd, expect_command_arg(l));
	}

	return ifcmd;
}

static struct ast_node *expect_command_classFunction(struct avdl_lexer *l) {

	struct ast_node *classFunc = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(classFunc, 0);
	ast_setLex(classFunc, "class_function");

	struct ast_node *classname = expect_identifier(l);
	struct ast_node *function = expect_command_functionDefinition(l);

	struct entry *classnameEntry = symtable_lookupEntry(ast_getLex(classname));
	if (!classnameEntry) {
		semantic_error(l, "cannot find class '%s' in symbol table.", ast_getLex(classname));
	}

	symtable_push();
	struct entry *e = symtable_entryat(symtable_insert("this", DD_VARIABLE_TYPE_STRUCT));
	e->isRef = 1;
	e->value = struct_table_get_index(ast_getLex(classname));

	// function arguments
	struct ast_node *funcargs = avdl_da_get(&function->children, 2);
	for (int i = 1; i < funcargs->children.elements; i += 2) {
		struct ast_node *type = avdl_da_get(&funcargs->children, i-1);
		struct ast_node *name = avdl_da_get(&funcargs->children, i);

		struct entry *symEntry = symtable_entryat(symtable_insert(ast_getLex(name), dd_variable_type_convert(ast_getLex(type))));
		if (!dd_variable_type_isPrimitiveType(ast_getLex(type))) {
			symEntry->isRef = 1;
			symEntry->value = struct_table_get_index(ast_getLex(type));
		}
	}
	//symtable_print();

	struct ast_node *functionStatements = expect_command(l);
	symtable_pop();

	ast_addChild(classFunc, classname);
	ast_addChild(function, functionStatements);
	ast_addChild(classFunc, function);

	return classFunc;
}

static struct ast_node *expect_command_functionDefinition(struct avdl_lexer *l) {

	struct ast_node *function = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(function, 0);
	ast_setLex(function, "function");

	struct ast_node *optionalModifier = expect_identifier(l);
	if (strcmp(ast_getLex(optionalModifier), "ref") == 0) {
		function->isRef = 1;
		ast_delete(optionalModifier);
		optionalModifier = expect_identifier(l);
	}
	else
	if (strcmp(ast_getLex(optionalModifier), "extern") == 0) {
		// apply modifier
		function->isExtern = 1;

		// get new optional modifier
		ast_delete(optionalModifier);
		optionalModifier = expect_identifier(l);
	}

	struct ast_node *functionName = expect_identifier(l);
	struct ast_node *args = expect_command(l);

	ast_addChild(function, optionalModifier);
	ast_addChild(function, functionName);
	ast_addChild(function, args);

	return function;
}

static struct ast_node *expect_command_group(struct avdl_lexer *l) {

	struct ast_node *group = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(group, 0);
	ast_setLex(group, "group");

	while (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND
	&&     avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND_BRACKET) {
		ast_addChild(group, expect_command_arg(l));
	}

	return group;
}

static struct ast_node *getIdentifierArrayNode(struct ast_node *n) {
	for (int i = 0; i < n->children.elements; i++) {
		struct ast_node *child = avdl_da_get(&n->children, i);

		if (child->node_type == AST_GROUP) {
			return child;
		}
	}

	return 0;
}

static struct ast_node *expect_command_classDefinition(struct avdl_lexer *l) {

	struct ast_node *classname = expect_identifier(l);
	symtable_insert(ast_getLex(classname), DD_VARIABLE_TYPE_VOID);

	// subclass can be an identifier (name of the class) or `0` (no parent class)
	struct ast_node *subclassname;
	if (avdl_lexer_peek(l) == LEXER_TOKEN_IDENTIFIER) {
		subclassname = expect_identifier(l);
	}
	// no subclass for this class
	else {
		struct ast_node *n = expect_int(l);
		if (n->value != 0) {
			semantic_error(l, "subclass can either be an identifier or '0'");
		}
		subclassname = ast_create(AST_EMPTY);
		ast_setValuei(subclassname, 0);
	}

	symtable_push();

	// add new struct to the struct table
	int structIndex;
	if (subclassname->node_type == AST_IDENTIFIER) {
		structIndex = struct_table_push(ast_getLex(classname), ast_getLex(subclassname));
	}
	else {
		structIndex = struct_table_push(ast_getLex(classname), 0);
	}

	struct ast_node *definitions = expect_command(l);

	for (int i = 0; i < definitions->children.elements; i++) {
		struct ast_node *child = avdl_da_get(&definitions->children, i);
		struct ast_node *type = avdl_da_get(&child->children, 0);
		struct ast_node *name = avdl_da_get(&child->children, 1);

		// new variable
		if (strcmp(ast_getLex(child), "def") == 0) {

			struct entry *e = symtable_entryat(symtable_lookup(ast_getLex(name)));

			struct ast_node *arrayNode = getIdentifierArrayNode(name);
			//printf("variable: %s %s\n", ast_getLex(type), ast_getLex(name));
			if (arrayNode) {

				if (arrayNode->children.elements == 0) {
					semantic_error(l, "array definition should have a value");
				}

				struct ast_node *arrayNum = avdl_da_get(&arrayNode->children, 0);

				if (arrayNum->node_type != AST_NUMBER) {
					semantic_error(l, "array definition should only be a number");
				}
				struct_table_push_member_array(ast_getLex(name), dd_variable_type_convert(ast_getLex(type)), ast_getLex(type), arrayNum->value, e->isRef);
			}
			else {
				struct_table_push_member(ast_getLex(name), dd_variable_type_convert(ast_getLex(type)), ast_getLex(type), e->isRef);
			}
		}
		// new function
		else {
			/*
			 * there's a subclass, check if new function is
			 * overriding another one
			 */
			if (subclassname->node_type == AST_IDENTIFIER) {
				int parentDepth = struct_table_is_member_parent(structIndex, ast_getLex(name));

				// overriding subclass function
				if (parentDepth >= 0) {
					child->value = 1;
				}
			}
			if (child->isRef) {
				//avdl_log("is ref: %s %d", ast_getLex(child), child->isRef);
			}

			// add function to struct table
			struct_table_push_member(ast_getLex(name), DD_VARIABLE_TYPE_FUNCTION, 0, child->isRef);
		}
	}
	symtable_pop();

	/* scan definitions
	 * if a function was defined in any of the subclasses, mark is
	 * 	as an override
	 */

	struct ast_node *classDefinition = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(classDefinition, 0);
	ast_setLex(classDefinition, "class");
	ast_addChild(classDefinition, classname);
	ast_addChild(classDefinition, subclassname);
	ast_addChild(classDefinition, definitions);
	return classDefinition;
}

static struct ast_node *expect_command_enum(struct avdl_lexer *l) {

	struct ast_node *enumname = expect_identifier(l);
	//symtable_insert(ast_getLex(classname), DD_VARIABLE_TYPE_VOID);

	struct ast_node *enumDefinition = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(enumDefinition, 0);
	ast_setLex(enumDefinition, "enum");
	ast_addChild(enumDefinition, enumname);
	while (avdl_lexer_peek(l) == LEXER_TOKEN_IDENTIFIER) {
		struct ast_node *child = expect_identifier(l);
		ast_addChild(enumDefinition, child);
	}

	return enumDefinition;
}

static struct ast_node *expect_command_struct(struct avdl_lexer *l) {

	struct ast_node *classname = expect_identifier(l);
	symtable_insert(ast_getLex(classname), DD_VARIABLE_TYPE_VOID);

	// subclass can be an identifier (name of the class) or `0` (no parent class)
	struct ast_node *subclassname;
	if (avdl_lexer_peek(l) == LEXER_TOKEN_IDENTIFIER) {
		subclassname = expect_identifier(l);
	}
	// no subclass for this class
	else {
		struct ast_node *n = expect_int(l);
		if (n->value != 0) {
			semantic_error(l, "subclass can either be an identifier or '0'");
		}
		subclassname = ast_create(AST_EMPTY);
		ast_setValuei(subclassname, 0);
	}

	symtable_push();

	// add new struct to the struct table
	int structIndex;
	structIndex = struct_table_push(ast_getLex(classname), 0);
	struct_table_SetStruct(structIndex);

	struct ast_node *definitions = expect_command(l);

	for (int i = 0; i < definitions->children.elements; i++) {
		struct ast_node *child = avdl_da_get(&definitions->children, i);
		struct ast_node *type = avdl_da_get(&child->children, 0);
		struct ast_node *name = avdl_da_get(&child->children, 1);

		// new variable
		if (strcmp(ast_getLex(child), "def") == 0) {

			struct entry *e = symtable_entryat(symtable_lookup(ast_getLex(name)));

			struct ast_node *arrayNode = getIdentifierArrayNode(name);
			//printf("variable: %s %s\n", ast_getLex(type), ast_getLex(name));
			if (arrayNode) {

				if (arrayNode->children.elements == 0) {
					semantic_error(l, "array definition should have a value");
				}

				struct ast_node *arrayNum = avdl_da_get(&arrayNode->children, 0);

				if (arrayNum->node_type != AST_NUMBER) {
					semantic_error(l, "array definition should only be a number");
				}
				struct_table_push_member_array(ast_getLex(name), dd_variable_type_convert(ast_getLex(type)), ast_getLex(type), arrayNum->value, e->isRef);
			}
			else {
				struct_table_push_member(ast_getLex(name), dd_variable_type_convert(ast_getLex(type)), ast_getLex(type), e->isRef);
			}
		}
	}
	symtable_pop();

	/* scan definitions
	 * if a function was defined in any of the subclasses, mark is
	 * 	as an override
	 */

	struct ast_node *classDefinition = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(classDefinition, 0);
	ast_setLex(classDefinition, "struct");
	ast_addChild(classDefinition, classname);
	ast_addChild(classDefinition, subclassname);
	ast_addChild(classDefinition, definitions);
	return classDefinition;
}

static struct ast_node *expect_command_definition(struct avdl_lexer *l) {

	struct ast_node *definition = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(definition, 0);
	ast_setLex(definition, "def");

	int isRef = 0;
	int isExtern = 0;

	// optional modifiers
	struct ast_node *optionalModifier = expect_identifier(l);
	do {
		if (strcmp(ast_getLex(optionalModifier), "ref") == 0) {
			// apply modifier
			isRef = 1;

			// get new optional modifier
			ast_delete(optionalModifier);
			optionalModifier = expect_identifier(l);
		}
		else
		if (strcmp(ast_getLex(optionalModifier), "extern") == 0) {
			// apply modifier
			isExtern = 1;

			// get new optional modifier
			ast_delete(optionalModifier);
			optionalModifier = expect_identifier(l);
		}
		else {
			break;
		}
	} while (1);

	definition->isExtern = isExtern;

	// get type
	struct ast_node *type = optionalModifier;

	// check if primitive or struct
	if (!dd_variable_type_isPrimitiveType(ast_getLex(type))
	&&  !struct_table_exists(ast_getLex(type))) {
		semantic_error(l, "unrecognized type '%s'", ast_getLex(type));
	}

	// get variable name
	struct ast_node *varname = expect_identifier(l);

	// add newly defined variable to symbol table
	struct entry *e = symtable_entryat(symtable_insert(ast_getLex(varname), dd_variable_type_convert(ast_getLex(type))));
	e->value = 0;
	definition->isStruct = 0;
	if (struct_table_exists(ast_getLex(type))) {
		e->value = struct_table_get_index(ast_getLex(type));
		definition->isStruct = struct_table_IsStruct(struct_table_get_index(ast_getLex(type)));;
	}
	e->isRef = isRef;
	definition->isRef = isRef;

	ast_addChild(definition, type);
	ast_addChild(definition, varname);

	if (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
		struct ast_node *initialValue = expect_command_arg(l);
		ast_addChild(definition, initialValue);
	}

	return definition;
}

static struct ast_node *expect_command_definitionShort(struct avdl_lexer *l, struct ast_node *n) {

	struct ast_node *definition = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(definition, 0);
	ast_setLex(definition, "def");
	definition->isExtern = 0;

	// get type
	struct ast_node *type = n;

	// check if primitive or struct
	if (!dd_variable_type_isPrimitiveType(ast_getLex(type))
	&&  !struct_table_exists(ast_getLex(type))) {
		semantic_error(l, "unrecognized type '%s'", ast_getLex(type));
	}

	// get variable name
	struct ast_node *varname = expect_identifier(l);

	// add newly defined variable to symbol table
	struct entry *e = symtable_entryat(symtable_insert(ast_getLex(varname), dd_variable_type_convert(ast_getLex(type))));
	e->value = 0;
	if (struct_table_exists(ast_getLex(type))) {
		e->value = struct_table_get_index(ast_getLex(type));
	}
	e->isRef = 0;
	definition->isRef = 0;

	ast_addChild(definition, type);
	ast_addChild(definition, varname);

	if (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
		struct ast_node *initialValue = expect_command_arg(l);
		ast_addChild(definition, initialValue);
	}

	return definition;
}

static struct ast_node *expect_int(struct avdl_lexer *l) {

	// confirm it's an integer
	if (avdl_lexer_getNextToken(l) != LEXER_TOKEN_INT) {
		semantic_error(l, "expected integer instead of '%s'", avdl_lexer_getLexToken(l));
	}

	struct ast_node *integer = ast_create(AST_NUMBER);
	ast_setValuei(integer, atoi(avdl_lexer_getLexToken(l)));
	return integer;
}

static struct ast_node *expect_float(struct avdl_lexer *l) {

	// confirm it's a float
	if (avdl_lexer_getNextToken(l) != LEXER_TOKEN_FLOAT) {
		semantic_error(l, "expected float instead of '%s'", avdl_lexer_getLexToken(l));
	}

	struct ast_node *f = ast_create(AST_FLOAT);
	ast_setValuef(f, atof(avdl_lexer_getLexToken(l)));
	return f;
}

static struct ast_node *expect_string(struct avdl_lexer *l) {

	// confirm it's a string
	if (avdl_lexer_getNextToken(l) != LEXER_TOKEN_STRING) {
		semantic_error(l, "expected string instead of '%s'", avdl_lexer_getLexToken(l));
	}

	struct ast_node *str = ast_create(AST_STRING);
	ast_setValuei(str, 0);
	ast_setLex(str, avdl_lexer_getLexToken(l));
	return str;
}

static struct ast_node *expect_command_binaryOperation(struct avdl_lexer *l, const char *binaryOperationLex) {

	struct ast_node *binaryOperation = ast_create(AST_COMMAND_NATIVE);
	ast_setValuei(binaryOperation, 0);
	ast_setLex(binaryOperation, binaryOperationLex);

	while (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
		ast_addChild(binaryOperation, expect_command_arg(l));
	}

	return binaryOperation;
}

static struct ast_node *expect_identifier(struct avdl_lexer *l) {

	// confirm it's an identifier
	if (avdl_lexer_getNextToken(l) != LEXER_TOKEN_IDENTIFIER) {
		semantic_error(l, "expected identifier instead of '%s'", avdl_lexer_getLexToken(l));
	}

	// generate ast node for it
	struct ast_node *identifier = ast_create(AST_IDENTIFIER);
	ast_setValuei(identifier, 0);
	ast_setLex(identifier, avdl_lexer_getLexToken(l));

	struct entry *symEntry = symtable_lookupEntry(ast_getLex(identifier));
	if (symEntry) {
		identifier->isRef = symEntry->isRef;
		identifier->value = symEntry->token;
	}

	// does it have an array modifier?
	if (avdl_lexer_peek(l) == LEXER_TOKEN_ARRAYSTART) {
		avdl_lexer_getNextToken(l);
		struct ast_node *array = ast_create(AST_GROUP);
		ast_setValuei(array, 0);
		int token = avdl_lexer_peek(l);
		// integer as array modifier
		if (token == LEXER_TOKEN_INT) {
			ast_addChild(array, expect_int(l));
		}
		else
		// identifier as array modifier
		if (token == LEXER_TOKEN_IDENTIFIER) {
			ast_addChild(array, expect_identifier(l));
		}
		else
		// calculation as array modifier
		if (token == LEXER_TOKEN_COMMANDSTART) {
			avdl_lexer_getNextToken(l);
			while (avdl_lexer_peek(l) == LEXER_TOKEN_COMMANDSTART) {
				ast_addChild(array, expect_command(l));
			}
		}
		ast_addChild(identifier, array);

		// end of array
		if (avdl_lexer_getNextToken(l) != LEXER_TOKEN_ARRAYEND) {
			semantic_error(l, "expected end of array");
		}
	}

	// it has a period (myclass.myvar)
	if (avdl_lexer_peek(l) == LEXER_TOKEN_PERIOD) {
		avdl_lexer_getNextToken(l);

		// identifiers that own objects have to be in the symbol table
		if (!symEntry) {
			semantic_error(l, "identifier '%s' not a known symbol", ast_getLex(identifier));
		}

		// identifiers that own objects have to be structs
		if (symEntry->token != DD_VARIABLE_TYPE_STRUCT) {
			semantic_error(l, "identifier '%s' not a struct, so it can't own objects", ast_getLex(identifier));
		}

		// add struct's members to new symbol table
		symtable_push();
		for (unsigned int j = 0; j < struct_table_get_member_total(symEntry->value); j++) {
			/*
			printf("	member: %s %d %s\n",
				struct_table_get_member_name(e->value, j),
				struct_table_get_member_type(e->value, j),
				struct_table_get_member_nametype(e->value, j)
			);
			*/

			struct entry *e2 = symtable_entryat(symtable_insert(
				struct_table_get_member_name(symEntry->value, j),
				struct_table_get_member_type(symEntry->value, j))
			);
			if (struct_table_get_member_type(symEntry->value, j) == DD_VARIABLE_TYPE_STRUCT) {
				e2->value = struct_table_get_index(struct_table_get_member_nametype(symEntry->value, j));
			}
			e2->isRef = struct_table_getMemberIsRef(symEntry->value, j);
		}

		// get the owned object's name
		struct ast_node *child = expect_identifier(l);

		// child not inside current symbol table - possibly belongs to a parent class
		int childSymId = symtable_lookup(ast_getLex(child));
		if (childSymId < 0) {
			int parentDepth = struct_table_is_member_parent(symEntry->value, ast_getLex(child));

			// child not part of that struct, or any of its parent classes
			if (parentDepth < 0) {
				semantic_error(l, "variable '%s' of class '%s' does not own object '%s'",
					ast_getLex(identifier), struct_table_get_name(symEntry->value), ast_getLex(child)
				);
			}
			else
			// child direct child of that class, but not in symbol table, this should never happen
			if (parentDepth == 0) {
				semantic_error(l, "variable '%s' of class '%s' owns object '%s', but couldn't be found in the symbol table",
					ast_getLex(identifier), struct_table_get_name(symEntry->value), ast_getLex(child)
				);
			}

			// add "parent" children in ast, for each parent class depth level
			struct ast_node *lastChild = identifier;
			for (int i = 0; i < parentDepth; i++) {
				struct ast_node *parentChild = ast_create(AST_IDENTIFIER);
				ast_setValuei(parentChild, 0);
				ast_setLex(parentChild, "parent");
				ast_addChild(lastChild, parentChild);
				int childIndex = ast_getChildCount(lastChild) -1;
				lastChild = avdl_da_get(&lastChild->children, childIndex);
			}

			// finally add the found child on the last "parent" node added
			ast_addChild(lastChild, child);
		}
		// child is directly owned by this struct, just add it as child
		else {
			if (struct_table_get_member_type(symEntry->value, struct_table_get_member(symEntry->value, ast_getLex(child))) == DD_VARIABLE_TYPE_STRUCT) {
				child->value = DD_VARIABLE_TYPE_STRUCT;
			}
			ast_addChild(identifier, child);
		}

		symtable_pop();
	}

	return identifier;
}

static struct ast_node *expect_command_arg(struct avdl_lexer *l) {

	int token = avdl_lexer_peek(l);
	if (token == LEXER_TOKEN_INT) {
		return expect_int(l);
	}
	else
	if (token == LEXER_TOKEN_FLOAT) {
		return expect_float(l);
	}
	else
	if (token == LEXER_TOKEN_STRING) {
		return expect_string(l);
	}
	else
	if (token == LEXER_TOKEN_COMMANDSTART
	||  token == LEXER_TOKEN_COMMANDSTART_BRACKET) {
		return expect_command(l);
	}
	else
	if (token == LEXER_TOKEN_IDENTIFIER) {
		return expect_identifier(l);
	}
	else {
		avdl_lexer_getNextToken(l);
		semantic_error(l, "expected command argument instead of '%s'", avdl_lexer_getLexToken(l));
	}

	return 0;
}

// looks for the structure of a command: (cmdname arg1 arg2 .. argN)
static struct ast_node *expect_command(struct avdl_lexer *l) {

	// confirm that a command is expected
	int commandStartToken = avdl_lexer_getNextToken(l);
	if (commandStartToken != LEXER_TOKEN_COMMANDSTART
	&&  commandStartToken != LEXER_TOKEN_COMMANDSTART_BRACKET) {
		semantic_error(l, "expected the start of a command '(', instead of '%s'", avdl_lexer_getLexToken(l));
	}

	// get the command's name
	struct ast_node *cmd;
	struct ast_node *cmdname;

	// empty command is equivalent to `(group)`
	if (avdl_lexer_peek(l) == LEXER_TOKEN_COMMANDEND) {
		cmdname = ast_create(AST_IDENTIFIER);
		ast_setValuei(cmdname, 0);
		ast_setLex(cmdname, "group");
	}
	else
	// group bracket command
	if (commandStartToken == LEXER_TOKEN_COMMANDSTART_BRACKET) {
		cmdname = ast_create(AST_IDENTIFIER);
		ast_setValuei(cmdname, 0);
		ast_setLex(cmdname, "group");
	}
	// otherwise, get command name
	else {
		cmdname = expect_identifier(l);
	}

	// native command, special rules
	if (agc_commands_isNative(ast_getLex(cmdname))) {

		// native command can only be a name, no array modifiers or owning data
		if (cmdname->children.elements > 0) {
			semantic_error(l, "native command name cannot have an array modifier, or own other data\n");
		}

		// based on the command, expect different data
		if (strcmp(ast_getLex(cmdname), "def") == 0) {
			cmd = expect_command_definition(l);
		}
		else
		if (dd_variable_type_isPrimitiveType(ast_getLex(cmdname))) {
			cmd = expect_command_definitionShort(l, cmdname);
		}
		else
		if (strcmp(ast_getLex(cmdname), "enum") == 0) {
			cmd = expect_command_enum(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "struct") == 0) {
			cmd = expect_command_struct(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "class") == 0) {
			cmd = expect_command_classDefinition(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "group") == 0) {
			cmd = expect_command_group(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "function") == 0) {
			cmd = expect_command_functionDefinition(l);

			symtable_push();
			struct ast_node *args = ast_getChild(cmd, 2);
			for (int i = 0; i < ast_getChildCount(args); i += 2) {
				struct ast_node *idtype = ast_getChild(args, i);
				struct ast_node *id = ast_getChild(args, i+1);
				struct entry *e = symtable_entryat(symtable_insert(ast_getLex(id), DD_VARIABLE_TYPE_STRUCT));
				e->isRef = 1;
				e->value = struct_table_get_index(ast_getLex(idtype));
			}

			if (avdl_lexer_peek(l) == LEXER_TOKEN_COMMANDSTART
			||  avdl_lexer_peek(l) == LEXER_TOKEN_COMMANDSTART_BRACKET) {
				// function statements
				ast_addChild(cmd, expect_command(l));
			}
			symtable_pop();

		}
		else
		if (strcmp(ast_getLex(cmdname), "class_function") == 0) {
			cmd = expect_command_classFunction(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "=") == 0
		||  strcmp(ast_getLex(cmdname), "+") == 0
		||  strcmp(ast_getLex(cmdname), "-") == 0
		||  strcmp(ast_getLex(cmdname), "*") == 0
		||  strcmp(ast_getLex(cmdname), "/") == 0
		||  strcmp(ast_getLex(cmdname), "%") == 0
		||  strcmp(ast_getLex(cmdname), "&&") == 0
		||  strcmp(ast_getLex(cmdname), "||") == 0
		||  strcmp(ast_getLex(cmdname), "==") == 0
		||  strcmp(ast_getLex(cmdname), ">=") == 0
		||  strcmp(ast_getLex(cmdname), "<=") == 0
		||  strcmp(ast_getLex(cmdname), ">") == 0
		||  strcmp(ast_getLex(cmdname), "<") == 0
		||  strcmp(ast_getLex(cmdname), "+=") == 0
		||  strcmp(ast_getLex(cmdname), "-=") == 0
		||  strcmp(ast_getLex(cmdname), "*=") == 0
		||  strcmp(ast_getLex(cmdname), "/=") == 0
		) {
			cmd = expect_command_binaryOperation(l, ast_getLex(cmdname));
		}
		else
		if (strcmp(ast_getLex(cmdname), "echo") == 0) {
			cmd = expect_command_group(l);
			ast_setLex(cmd, "echo");
		}
		else
		if (strcmp(ast_getLex(cmdname), "log") == 0) {
			cmd = expect_command_group(l);
			ast_setLex(cmd, "log");
		}
		else
		if (strcmp(ast_getLex(cmdname), "log_error") == 0) {
			cmd = expect_command_group(l);
			ast_setLex(cmd, "log_error");
		}
		else
		if (strcmp(ast_getLex(cmdname), "if") == 0) {
			cmd = expect_command_if(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "include") == 0) {
			cmd = expect_command_include(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "asset") == 0) {
			cmd = expect_command_asset(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "savefile") == 0) {
			cmd = expect_command_savefile(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "for") == 0) {
			cmd = expect_command_for(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "break") == 0) {
			cmd = expect_command_break(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "continue") == 0) {
			cmd = expect_command_continue(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "multistring") == 0) {
			cmd = expect_command_multistring(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "unicode") == 0) {
			cmd = expect_command_unicode(l);
		}
		else
		if (strcmp(ast_getLex(cmdname), "return") == 0) {
			cmd = expect_command_return(l);
		}
		else {
			semantic_error(l, "no rule to parse command '%s'", ast_getLex(cmdname));
		}
	}
	// custom command, make sure it exists
	else {
		/*
		if (symtable_lookup(ast_getLex(cmdname)) == -1) {
			printf("unrecognized identifier: %s\n", ast_getLex(cmdname));
			exit(-1);
		}
		*/
		cmd = ast_create(0);
		ast_setValuei(cmd, 0);
		cmd->node_type = AST_COMMAND_CUSTOM;
		ast_addChild(cmd, cmdname);

		while (avdl_lexer_peek(l) != LEXER_TOKEN_COMMANDEND) {
			ast_addChild(cmd, expect_command_arg(l));
		}
	}

	// get the command's children
	int commandEndToken = avdl_lexer_getNextToken(l);
	if (commandStartToken == LEXER_TOKEN_COMMANDSTART) {
		if (commandEndToken != LEXER_TOKEN_COMMANDEND) {
			semantic_error(l, "expected command end ')'");
		}
	}
	else
	if (commandStartToken == LEXER_TOKEN_COMMANDSTART_BRACKET) {
		if (commandEndToken != LEXER_TOKEN_COMMANDEND_BRACKET) {
			semantic_error(l, "expected command end '}'");
		}
	}

	return cmd;
}

static struct avdl_vec3 {
	float v[3];
};

static struct ast_node *json_expect_array3f(struct avdl_json_object *json, struct avdl_vec3 *v) {

	if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
		avdl_log_error("Json: expected array start for 3f: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return 0;
	}

	int index = 0;

	avdl_json_next(json);
	for (int i = 0; i < 3; i++) {
		if (avdl_json_getToken(json) != AVDL_JSON_FLOAT) {
			avdl_log_error("Json 3f: was expecting 3 floats but found something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return 0;
		}

		v->v[i] = avdl_json_getTokenFloat(json);
		avdl_json_next(json);
	}
}

static struct ast_node *json_expect_component(struct avdl_json_object *json, struct ast_node *parent) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log_error("Json component should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return 0;
	}

	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {
		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log_error("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return 0;
		}
		//avdl_log("got component key: %s", avdl_json_getTokenString(json));

		avdl_json_next(json);
		if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
			//avdl_log("got component string: %s", avdl_json_getTokenString(json));
		}
		else {
			//avdl_log("component something else?");
		}

		avdl_json_next(json);
	}

	avdl_json_next(json);

}

static int transform_counter = 0;

static struct ast_node *json_expect_node(struct avdl_json_object *json, struct ast_node *parent, char *node_parent_name) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log_error("Json should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return 0;
	}

	// generate node name
	char node_name[100];
	strcpy(node_name, "n_");
	snprintf(node_name +2, 80, "%d", transform_counter);

	if (transform_counter > 0) {

		// every node apart from the first one needs to have a parent
		if (!node_parent_name) {
			avdl_log_error("node parent name missing on non-root node");
			return 0;
		}

		// define node
		struct ast_node *node_def = ast_create(AST_COMMAND_NATIVE);
		ast_setLex(node_def, "def");
		node_def->isRef = 1;
		struct ast_node *node_def_c1 = ast_create(AST_IDENTIFIER);
		ast_setLex(node_def_c1, "avdl_node");
		struct ast_node *node_def_c2 = ast_create(AST_IDENTIFIER);
		ast_setLex(node_def_c2, node_name);

		ast_addChild(node_def, node_def_c1);
		ast_addChild(node_def, node_def_c2);
		ast_addChild(parent, node_def);

		// add child to parent and assign
		struct ast_node *node_assign = ast_create(AST_COMMAND_NATIVE);
		ast_setLex(node_assign, "=");
		struct ast_node *node_assign_id = ast_create(AST_IDENTIFIER);
		ast_setLex(node_assign_id, node_name);
		struct ast_node *node_create_child = ast_create(AST_COMMAND_CUSTOM);

			struct ast_node *node_create_child_id = ast_create(AST_IDENTIFIER);
			ast_setLex(node_create_child_id, node_parent_name);
			node_create_child_id->isRef = 1;
			struct ast_node *node_create_child_id_child = ast_create(AST_IDENTIFIER);
			ast_setLex(node_create_child_id_child, "AddChild");
			ast_addChild(node_create_child_id, node_create_child_id_child);

		ast_addChild(node_create_child, node_create_child_id);

		struct ast_node *node_create_child_id_child2 = ast_create(AST_IDENTIFIER);
		ast_setLex(node_create_child_id_child2, node_parent_name);
		ast_addChild(node_create_child, node_create_child_id_child2);

		ast_addChild(node_assign, node_assign_id);
		ast_addChild(node_assign, node_create_child);
		ast_addChild(parent, node_assign);
	}

	// generate transform name
	char transform_name[100];
	strcpy(transform_name, "t_");
	snprintf(transform_name +2, 80, "%d", transform_counter);

	{
		// define transform node
		struct ast_node *transform_def = ast_create(AST_COMMAND_NATIVE);
		ast_setLex(transform_def, "def");
		transform_def->isRef = 1;
		struct ast_node *transform_def_node_c1 = ast_create(AST_IDENTIFIER);
		ast_setLex(transform_def_node_c1, "avdl_transform");
		struct ast_node *transform_def_node_c2 = ast_create(AST_IDENTIFIER);
		ast_setLex(transform_def_node_c2, transform_name);

		ast_addChild(transform_def, transform_def_node_c1);
		ast_addChild(transform_def, transform_def_node_c2);
		ast_addChild(parent, transform_def);

		// assign transform node
		struct ast_node *node_assign = ast_create(AST_COMMAND_NATIVE);
		ast_setLex(node_assign, "=");
		struct ast_node *node_assign_id = ast_create(AST_IDENTIFIER);
		ast_setLex(node_assign_id, transform_name);
		struct ast_node *node_create_child = ast_create(AST_COMMAND_CUSTOM);

			struct ast_node *node_create_child_id = ast_create(AST_IDENTIFIER);
			ast_setLex(node_create_child_id, node_name);
			node_create_child_id->isRef = 1;
			struct ast_node *node_create_child_id_child = ast_create(AST_IDENTIFIER);
			ast_setLex(node_create_child_id_child, "GetLocalTransform");
			ast_addChild(node_create_child_id, node_create_child_id_child);

		ast_addChild(node_create_child, node_create_child_id);

		struct ast_node *node_create_child_id_child2 = ast_create(AST_IDENTIFIER);
		ast_setLex(node_create_child_id_child2, node_name);
		ast_addChild(node_create_child, node_create_child_id_child2);

		ast_addChild(node_assign, node_assign_id);
		ast_addChild(node_assign, node_create_child);
		ast_addChild(parent, node_assign);
	}

	transform_counter++;

	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {

		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log_error("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return 0;
		}
		//avdl_log("got key: %s", avdl_json_getTokenString(json));

		if (strcmp(avdl_json_getTokenString(json), "name") == 0) {
			avdl_json_next(json);
			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				//avdl_log("got string name: %s", avdl_json_getTokenString(json));

				struct ast_node *node_set_name = ast_create(AST_COMMAND_CUSTOM);

					struct ast_node *node_set_name_id = ast_create(AST_IDENTIFIER);
					ast_setLex(node_set_name_id, node_name);
					node_set_name_id->isRef = 1;
					struct ast_node *node_set_name_id2 = ast_create(AST_IDENTIFIER);
					ast_setLex(node_set_name_id2, "SetName");
					ast_addChild(node_set_name_id, node_set_name_id2);

				ast_addChild(node_set_name, node_set_name_id);

				// arguments
				struct ast_node *node_arg1 = ast_create(AST_IDENTIFIER);
				ast_setLex(node_arg1, node_name);
				ast_addChild(node_set_name, node_arg1);

				struct ast_node *node_arg2 = ast_create(AST_STRING);
				ast_setLex(node_arg2, avdl_json_getTokenString(json));
				ast_addChild(node_set_name, node_arg2);

				ast_addChild(parent, node_set_name);
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "position") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got position: %f %f %f", v.v[0], v.v[1], v.v[2]);

			struct ast_node *node_position = ast_create(AST_COMMAND_CUSTOM);

				struct ast_node *node_position_id = ast_create(AST_IDENTIFIER);
				ast_setLex(node_position_id, transform_name);
				node_position_id->isRef = 1;
				struct ast_node *node_position_id2 = ast_create(AST_IDENTIFIER);
				ast_setLex(node_position_id2, "SetPosition3f");
				ast_addChild(node_position_id, node_position_id2);

			ast_addChild(node_position, node_position_id);

			// arguments
			struct ast_node *node_arg1 = ast_create(AST_IDENTIFIER);
			ast_setLex(node_arg1, transform_name);
			ast_addChild(node_position, node_arg1);

			struct ast_node *node_arg2 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg1, v.v[0]);
			ast_addChild(node_position, node_arg2);

			struct ast_node *node_arg3 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg3, v.v[1]);
			ast_addChild(node_position, node_arg3);

			struct ast_node *node_arg4 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg4, v.v[2]);
			ast_addChild(node_position, node_arg4);

			ast_addChild(parent, node_position);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "rotation") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got rotation: %f %f %f", v.v[0], v.v[1], v.v[2]);

			struct ast_node *node_rotation = ast_create(AST_COMMAND_CUSTOM);

				struct ast_node *node_rotation_id = ast_create(AST_IDENTIFIER);
				ast_setLex(node_rotation_id, transform_name);
				node_rotation_id->isRef = 1;
				struct ast_node *node_rotation_id2 = ast_create(AST_IDENTIFIER);
				ast_setLex(node_rotation_id2, "SetRotation3f");
				ast_addChild(node_rotation_id, node_rotation_id2);

			ast_addChild(node_rotation, node_rotation_id);

			// arguments
			struct ast_node *node_arg1 = ast_create(AST_IDENTIFIER);
			ast_setLex(node_arg1, transform_name);
			ast_addChild(node_rotation, node_arg1);

			struct ast_node *node_arg2 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg1, v.v[0]);
			ast_addChild(node_rotation, node_arg2);

			struct ast_node *node_arg3 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg3, v.v[1]);
			ast_addChild(node_rotation, node_arg3);

			struct ast_node *node_arg4 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg4, v.v[2]);
			ast_addChild(node_rotation, node_arg4);

			ast_addChild(parent, node_rotation);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "scale") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got scale: %f %f %f", v.v[0], v.v[1], v.v[2]);

			struct ast_node *node_scale = ast_create(AST_COMMAND_CUSTOM);

				struct ast_node *node_scale_id = ast_create(AST_IDENTIFIER);
				ast_setLex(node_scale_id, transform_name);
				node_scale_id->isRef = 1;
				struct ast_node *node_scale_id2 = ast_create(AST_IDENTIFIER);
				ast_setLex(node_scale_id2, "SetScale3f");
				ast_addChild(node_scale_id, node_scale_id2);

			ast_addChild(node_scale, node_scale_id);

			// arguments
			struct ast_node *node_arg1 = ast_create(AST_IDENTIFIER);
			ast_setLex(node_arg1, transform_name);
			ast_addChild(node_scale, node_arg1);

			struct ast_node *node_arg2 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg1, v.v[0]);
			ast_addChild(node_scale, node_arg2);

			struct ast_node *node_arg3 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg3, v.v[1]);
			ast_addChild(node_scale, node_arg3);

			struct ast_node *node_arg4 = ast_create(AST_FLOAT);
			ast_setValuef(node_arg4, v.v[2]);
			ast_addChild(node_scale, node_arg4);

			ast_addChild(parent, node_scale);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "components") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log_error("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return 0;
			}
		
			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				json_expect_component(json, parent);
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "children") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log_error("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return 0;
			}
		
			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				json_expect_node(json, parent, node_name);
			}
		}
		else {
			avdl_json_next(json);
			avdl_log("something else: %s", avdl_json_getTokenString(json));
			break;
		}
		avdl_json_next(json);
	}
	avdl_json_next(json);

	//return node;
	return 0;

}

static int semanticAnalyser_convertToAst_json_node(struct ast_node *node) {
	return 0;
}

static int semanticAnalyser_convertToAst_json(struct ast_node *node, const char *filename) {
	struct avdl_json_object json;
	//avdl_log("json parse");

	// get function name from filename
	char id[100];

	// filename should have the form "src/my_file.json", first remove everything before and including "/"
	char *p = strstr(filename, "/");
	if (p) {
		p++;
		strcpy(id, p);
	}
	else {
		strcpy(id, filename);
	}

	// remove anything after and including "."
	p = strstr(id, ".");
	if (p) {
		p[0] = '\0';
	}
	//avdl_log("filename: %s", filename);
	//avdl_log("id: %s", id);

	// init
	avdl_json_initFile(&json, filename);
	avdl_json_next(&json);

	// pre-made function template
	struct ast_node *func = ast_create(AST_COMMAND_NATIVE);
	ast_setLex(func, "function");

	// func type
	struct ast_node *functype = ast_create(AST_IDENTIFIER);
	ast_setLex(functype, "void");

	// func name
	struct ast_node *funcname = ast_create(AST_IDENTIFIER);
	ast_setLex(funcname, id);

	ast_addChild(func, functype);
	ast_addChild(func, funcname);

	// func args - default node
	struct ast_node *funcargs = ast_create(AST_COMMAND_NATIVE);
	ast_setLex(funcargs, "group");

	struct ast_node *defaultnode = ast_create(AST_IDENTIFIER);
	ast_setLex(defaultnode, "avdl_node");

	struct ast_node *defaultnodename = ast_create(AST_IDENTIFIER);
	ast_setLex(defaultnodename, "n_0");

	ast_addChild(funcargs, defaultnode);
	ast_addChild(funcargs, defaultnodename);

	ast_addChild(func, funcargs);

	// func statements
	struct ast_node *funcstatements = ast_create(AST_COMMAND_NATIVE);
	ast_setLex(funcstatements, "group");

	transform_counter = 0;
	json_expect_node(&json, funcstatements, 0);
	//avdl_log("json parse complete");

	ast_addChild(func, funcstatements);
	ast_addChild(node, func);
	//ast_print(node);

	// clean
	avdl_json_deinit(&json);
	return 0;
}

int semanticAnalyser_convertToAst(struct ast_node *node, const char *filename) {

	struct_table_init();
	symtable_init();

	struct avdl_lexer l;
	avdl_lexer_create(&l, filename);

	//avdl_log("filename: %s", filename);
	// json format
	if (strcmp(filename +strlen(filename) -5, ".json") == 0) {
		return semanticAnalyser_convertToAst_json(node, filename);
	}

	// check for error src
	while (avdl_lexer_peek(&l) != LEXER_TOKEN_COMMANDSTART
	&&     avdl_lexer_peek(&l) != LEXER_TOKEN_COMMANDEND_BRACKET) {
		avdl_log_error("src file is not a valid avdl program: %s", filename);
		return 0;
	}

	while (avdl_lexer_peek(&l) == LEXER_TOKEN_COMMANDSTART
	||     avdl_lexer_peek(&l) == LEXER_TOKEN_COMMANDEND_BRACKET) {
		struct ast_node *cmd = expect_command(&l);

		if (cmd->node_type == AST_INCLUDE) {
			if (avdl_lexer_addIncludedFile(&l, ast_getLex(cmd)) != 0) {
				return -1;
			}
		}
		else {
			ast_addChild(node, cmd);
		}
	}
	//ast_print(node);

	avdl_lexer_clean(&l);

	return 0;
}

static void semantic_error(struct avdl_lexer *l, const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	avdl_log_error("semantic analysis: syntax error at " BLU "%s:%d:%d" RESET, avdl_lexer_getCurrentFilename(l), avdl_lexer_getCurrentLinenumber(l), avdl_lexer_getCurrentCharacterNumber(l));
	//avdl_log(msg, args);
	vprintf(msg, args);
	printf("\n");
	avdl_lexer_printCurrentLine(l);

	va_end(args);
	exit(-1);
}
