%{
#include <stdio.h>
#include <string.h>
#include "yacc.tab.h"
#include "symtable.h"
#include "ast_node.h"
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "struct_table.h"
#include "dd_commands.h"
#include <errno.h>

extern FILE *yyin;
extern YYSTYPE yylex(void);

// line number (got from lex.l)
extern int linenum;

extern float parsing_float;

extern int include_stack_ptr;

char included_files[10][100];
int included_files_num = 0;

// buffer for general use
#define DD_BUFFER_SIZE 1000
char buffer[DD_BUFFER_SIZE];

// error
void yyerror(const char *str)
{
        fprintf(stderr,"error: line %d: %s\n", linenum, str);
	_exit(-1);
}

// game node, parent of all nodes
struct ast_node *game_node;

#define MAX_INPUT_FILES 10

// init data, parse, exit
int main(int argc, char *argv[])
{
	/* tweakable data
	 */
	int show_ast = 0;
	int show_struct_table = 0;
	int compile = 1;
	char *filename[MAX_INPUT_FILES];
	FILE *input_file = 0;
	int input_file_total = 0;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// print abstract syntax tree
		if (strcmp(argv[i], "--print-ast") == 0) {
			show_ast = 1;
		}
		else
		// print struct table
		if (strcmp(argv[i], "--print-struct-table") == 0) {
			show_struct_table = 1;
		}
		else
		// only translate (not compile)
		if (strcmp(argv[i], "--translate-only") == 0) {
			compile = 0;
		}
		else
		// input file
		if (input_file_total < MAX_INPUT_FILES) {
			filename[input_file_total] = argv[i];
			input_file_total++;
		}
		else {
			printf("avdl error: '%s': Only %d input files can be provided at a time\n", argv[i], MAX_INPUT_FILES);
			return -1;
		}
	}

	// make sure the minimum to parse exists
	if (input_file_total <= 0) {
		printf("avdl error: No filename given\n");
		return -1;
	}

	// compile all given input files
	for (int i = 0; i < input_file_total; i++) {

		linenum = 1;
		included_files_num = 0;
		include_stack_ptr = 0;

		input_file = fopen(filename[i], "r");
		if (!input_file) {
			printf("avdl error: Unable to open '%s': %s\n", filename[i], strerror(errno));
			return -1;
		}

		yyin = input_file;

		// init data
		linenum = 1;

		// initial symbols
		symtable_init();

		/* keywords
		 */
		for (unsigned int i = 0; i < keywords_total; i++) {
			symtable_insert(keywords[i].keyword, DD_KEYWORD);
		}
		/*
		symtable_insert("DD_WIDTH", DD_INTERNAL_WIDTH);
		symtable_insert("DD_HEIGHT", DD_INTERNAL_HEIGHT);
		*/

		// initialise the parent node
		game_node = ast_create(AST_GAME, 0);

		// init structs
		/*
		struct struct_entry *temp_entry = malloc(sizeof(struct struct_entry));
		temp_entry->name = "dd_world";
		struct_insert(temp_entry);
		temp_entry = malloc(sizeof(struct struct_entry));
		temp_entry->name = "dd_sprite";
		struct_insert(temp_entry);
		temp_entry = malloc(sizeof(struct struct_entry));
		temp_entry->name = "dd_vector2d";
		struct_insert(temp_entry);
		*/
		struct_table_init();
		struct_table_push("dd_world", 0);
		struct_table_push_member("create", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("resize", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("key_input", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push("dd_matrix", 0);
		struct_table_push("dd_mesh", 0);
		struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("set_primitive", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("copy", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push("dd_meshColour", "dd_mesh");
		struct_table_push_member("set_colour", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push("dd_meshTexture", "dd_meshColour");
		struct_table_push_member("loadTexture", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("set_primitive_texcoords", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push("dd_sound", 0);
		struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("play", DD_VARIABLE_TYPE_FUNCTION, 0);

		yyparse();
		fclose(input_file);

		// parse resulting ast tree to a file
		//parse_javascript("build/game.js", game_node);
		//parse_cglut("build-cglut/game.c", game_node);
		buffer[0] = '\0';
		sprintf(buffer, "build-cglut/%s.c", filename[i]);
		parse_cglut_translate_only(buffer, game_node, 0);

		if (show_ast) {
			ast_print(game_node);
		}

		for (int i = 0; i < included_files_num; i++) {
			include_stack_ptr = 0;
			symtable_clean();
			input_file = fopen(included_files[i], "r");
			if (!input_file) {
				printf("avdl error: Unable to open included file '%s': %s\n", included_files[i], strerror(errno));
				return -1;
			}

			sprintf(buffer, "build-cglut/%s.h", included_files[i]);

			// stat source and destination asset, if source is newer, copy to destination
			struct stat stat_src_asset;
			if (stat(included_files[i], &stat_src_asset) == -1) {
				printf("avdl error: Unable to stat '%s': %s\n", included_files[i], strerror(errno));
				exit(-1);
			}

			struct stat stat_dst_asset;
			if (stat(buffer, &stat_dst_asset) == -1) {
				yyin = input_file;
				game_node = ast_create(AST_GAME, 0);
				yyparse();
				fclose(input_file);
				parse_cglut_translate_only(buffer, game_node, 1);
				if (show_ast) {
					ast_print(game_node);
				}
				continue;
			}

			// check last modification time
			time_t time_src = mktime(gmtime(&stat_src_asset.st_mtime));
			time_t time_dst = mktime(gmtime(&stat_dst_asset.st_mtime));

			if (time_src > time_dst) {
				yyin = input_file;
				game_node = ast_create(AST_GAME, 0);
				yyparse();
				fclose(input_file);
				parse_cglut_translate_only(buffer, game_node, 1);
				if (show_ast) {
					ast_print(game_node);
				}
				continue;
			}
		}

		// print debug data
		/*
		if (show_ast) {
			ast_print(game_node);
		}
		*/

		if (show_struct_table) {
			struct_table_print();
		}

		//symtable_print();

		// clean symtable and ast tree
		symtable_clean();
		ast_delete(game_node);
	}

	// compile the final executable
	if (compile) {
		buffer[0] = '\0';
		sprintf(buffer, "gcc ");
		for (int i = 0; i < input_file_total; i++) {
			strcat(buffer, "build-cglut/");
			strcat(buffer, filename[i]);
			strcat(buffer, ".c ");
		}
		strcat(buffer, "-O3 -o build-cglut/game -lGL -lGLU -lGLEW -lglut -lddcglut -lm -w -lSDL2 -lSDL2_mixer");
		system(buffer);
	}

	// success!
	return 0;
}

%}

// nothingness
%token DD_ZERO

// used for the languages keywords
%token DD_KEYWORD

/* constants */
%token DD_CONSTANT_SYMBOL DD_CONSTANT_STRING DD_CONSTANT_NUMBER DD_CONSTANT_FLOAT DD_CONSTANT_INCLUDE

%%

/* each rule creates a node,
 * possibly with children nodes,
 * all non-terminals are nodes that can be obtained with ast_pop() (left to right)
 */

/* the game itself, contains commands
 */
game:
	commands {
		ast_child_add(game_node, ast_pop());
	}
	;

/* commands,
 * at least one, but can be more
 * returns AST_GROUP
 */
commands:
	/* single command, creates a group of itself
	 */
	command {
		struct ast_node *n = ast_create(AST_GROUP, 0);
		ast_child_add(n, ast_pop());
		ast_push(n);
	}
	|
	/* single command but more are following,
	 * add the command to the group
	 */
	command commands {
		struct ast_node *n = ast_pop();
		ast_child_add_first(n, ast_pop());
		ast_push(n);
	};

/* single command
 * has a keyword and optional arguments
 */
command:
	'(' cmd_name optional_args ')' {

		// get nodes
		struct ast_node *opt_args = ast_pop();
		struct ast_node *cmd_name = ast_pop();


		/*
		struct ast_node *cmd = parse_command(cmd_name, opt_args);
		//ast_print(cmd);

		ast_push(cmd);

		goto CONTINUE;
		*/

		// find out if keyword is an native keyword or a custom one
		struct entry *e = symtable_entryat(cmd_name->value);
		int type;

		if (e->token == DD_KEYWORD) {
			type = AST_COMMAND_NATIVE;
			if (strcmp(e->lexptr, "group") == 0) {
				type = AST_GROUP;
			}
			else
			if (strcmp(e->lexptr, "class") == 0) {
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
							if (!isFunction) {
								vartype = dd_da_get(&statement->children, 0);
								evartype = symtable_entryat(vartype->value);
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
							else {
								type = DD_VARIABLE_TYPE_STRUCT;
								nametype = evartype->lexptr;
							}

							// only add it to the struct table if not member of any of its parents
							/*
							if (struct_table_is_member_parent(
								struct_table_get_index(eclassname->lexptr),
								evarname->lexptr) == -1) {

								struct_table_push_member(evarname->lexptr, type, nametype);
							}
							*/
							struct_table_push_member(evarname->lexptr, type, nametype);
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
			}
			else
			if (strcmp(e->lexptr, "array") == 0) {
				struct ast_node *type = dd_da_get(&opt_args->children, 0);
				struct ast_node *amount = dd_da_get(&opt_args->children, 1);
				struct ast_node *n = ast_create(AST_IDENTIFIER, type->value);
				n->arraySize = amount->value;
				ast_push(n);
				goto CONTINUE;
			}
			else
			if (strcmp(e->lexptr, "ref") == 0) {
				struct ast_node *type = dd_da_get(&opt_args->children, 0);
				struct ast_node *n = ast_create(AST_IDENTIFIER, type->value);
				n->isRef = 1;
				ast_push(n);
				goto CONTINUE;
			}
			else
			if (strcmp(e->lexptr, "def") == 0) {
				struct ast_node *varname = dd_da_get(&opt_args->children, 1);
				struct entry *evarname = symtable_entryat(varname->value);
				struct ast_node *vartype = dd_da_get(&opt_args->children, 0);
				struct entry *evartype = symtable_entryat(vartype->value);
				evarname->scope = evartype->lexptr;
			}
		}
		// not a native keyword, assume custom one
		else {
			sprintf(buffer, "not a keyword: '%s'", e->lexptr);
			type = AST_COMMAND_CUSTOM;
			//yyerror(buffer);
	    		//printf("error symbol not keyword: %s\n", e->lexptr);
		}

		/* command node
		 * construct it in such a way that the parent ast node is the command
		 * if a custom command, then first child is its name
		 */
		opt_args->node_type = type;
		opt_args->value = $2;
		if (type == AST_COMMAND_CUSTOM) {
			ast_child_add_first(opt_args, cmd_name);
		}
		opt_args->isIncluded = include_stack_ptr != 0;
		ast_push(opt_args);

		CONTINUE:
		(void) opt_args->value;
	}
	|
	DD_CONSTANT_INCLUDE {
		//struct ast_node *group = ast_pop();
		//printf("cmd include\n");
		ast_push(ast_create(AST_INCLUDE, $1));
		//ast_push(group);
	};

/* optional args
 */
optional_args:
	arg optional_args {
		struct ast_node *n = ast_pop();
		struct ast_node *arg = ast_pop();
		ast_child_add_first(n, arg);
		ast_push(n);
	}
	|
	{
		ast_push(ast_create(AST_GROUP, 0));
	}
	;

/* argument
 */
arg:
	DD_CONSTANT_FLOAT {
		struct ast_node *n = ast_create(AST_FLOAT, 0);
		n->fvalue = parsing_float;
		ast_push(n);
	}
	|
   	DD_CONSTANT_NUMBER {
		struct ast_node *n = ast_create(AST_NUMBER, $1);
		ast_push(n);
	}
	|
   	DD_CONSTANT_STRING {
		struct ast_node *n = ast_create(AST_STRING, $1);
		ast_push(n);
	}
	|
	identifier {
		struct ast_node *n = ast_pop();
		ast_push(n);
	}
	|
	command
	;

/* command name
 * can be either one character or an identifier (chain of symbols)
 */
cmd_name:
	identifier
	;

/* identifier
 * can be a chain of symbols (chains with '.')
 * example: this.my_obj.x
 */
identifier:
	DD_CONSTANT_SYMBOL optional_array_index {
		struct ast_node *n = ast_create(AST_IDENTIFIER, $1);

		// check if an array, and add as a child
		if ($2) {
			struct ast_node *opt_index = ast_pop();
			ast_child_add(n, opt_index);
		}

		ast_push(n);
	}
	|
	identifier '.' DD_CONSTANT_SYMBOL optional_array_index {
		struct ast_node *new = ast_create(AST_IDENTIFIER, $3);

		// check if an array, and add as a child
		if ($4) {
			struct ast_node *opt_index = ast_pop();
			ast_child_add(new, opt_index);
		}

		struct ast_node *group = ast_pop();
		ast_child_add(group, new);
		ast_push(group);
	}
	;

optional_array_index: {
		$$ = 0;
	}
	|
	'[' optional_args ']' {
		$$ = 1;
		struct ast_node *n = ast_pop();
		ast_push(n);
	}
	;
