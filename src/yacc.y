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
#include "file_op.h"

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
	char filename[MAX_INPUT_FILES][100];
	FILE *input_file = 0;
	int input_file_total = 0;
	char *outname = 0;
	char *includePath = 0;

	/*
	 * phases
	 *
	 * translate: translate `.dd` files to `.c`
	 * compile: compile `.c` files to `.o`
	 * link: link all `.o` files to an executable
	 *
	 */
	int translate = 1;
	int compile = 1;
	int link = 1;

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
		/* phase arguments
		 * -c: skip linking
		 */
		if (strcmp(argv[i], "-c") == 0) {
			link = 0;
		}
		else
		if (strcmp(argv[i], "-o") == 0) {
			if (argc > i+1) {
				outname = argv[i+1];
				i++;
			}
			else {
				printf("avdl error: name is expected after `-o`\n");
				return -1;
			}
		}
		else
		if (strcmp(argv[i], "-I") == 0) {
			if (argc > i+1) {
				includePath = argv[i+1];
				i++;
			}
			else {
				printf("avdl error: include path is expected after `-I`\n");
				return -1;
			}
		}
		else
		// input file
		if (input_file_total < MAX_INPUT_FILES) {
			strncpy(filename[input_file_total], argv[i], 100);
			filename[input_file_total][99] = '\0';
			//filename[input_file_total] = argv[i];
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

	if (outname && !link && input_file_total > 1) {
		printf("avdl error: cannot supply `-o` with `-c` and multiple input files\n");
		return -1;
	}

	if (translate) {
		//printf("~~~ translation phase ~~~\n");
	}
	// translate all given input files
	for (int i = 0; i < input_file_total && translate; i++) {

		// check if file is meant to be compiled, or is already compiled
		if (strcmp(filename[i] +strlen(filename[i]) -3, ".dd") != 0) {
			continue;
		}

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
		struct_table_push_member("playLoop", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("stop", DD_VARIABLE_TYPE_FUNCTION, 0);
		struct_table_push_member("setVolume", DD_VARIABLE_TYPE_FUNCTION, 0);

		yyparse();
		fclose(input_file);

		//printf("transpiling: %s", filename[i]);
		// parse resulting ast tree to a file
		filename[i][strlen(filename[i]) -2] = 'c';
		filename[i][strlen(filename[i]) -1] = '\0';
		//printf(" to %s\n", filename[i]);
		parse_cglut_translate_only(filename[i], game_node, 0);

		// print debug data
		if (show_ast) {
			ast_print(game_node);
		}

		if (show_struct_table) {
			struct_table_print();
		}

		//symtable_print();

		// clean symtable and ast tree
		symtable_clean();
		ast_delete(game_node);

		for (int i = 0; i < included_files_num; i++) {
			//printf("transpiling included file: %s\n", included_files[i]);
			include_stack_ptr = 0;
			symtable_clean();
			input_file = fopen(included_files[i], "r");
			if (!input_file) {
				printf("avdl error: Unable to open included file '%s': %s\n", included_files[i], strerror(errno));
				return -1;
			}

			strcpy(buffer, included_files[i]);
			buffer[strlen(buffer)-3] = 'h';
			buffer[strlen(buffer)-2] = '\0';
			//printf("transpiling to: %s\n", buffer);
			//sprintf(buffer, "build-cglut/%s.h", included_files[i]);

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
	}

	if (compile) {
		//printf("~~~ compilation phase ~~~\n");
	}
	// compile all given input files
	for (int i = 0; i < input_file_total && compile; i++) {

		// check if file is meant to be compiled, or is already compiled
		if (strcmp(filename[i] +strlen(filename[i]) -2, ".c") != 0) {
			continue;
		}

		//printf("compiling: %s\n", filename[i]);

		// compile
		strcpy(buffer, "gcc -c -w ");
		strcat(buffer, filename[i]);
		strcat(buffer, " -o ");
		if (outname && !link) {
			strcat(buffer, outname);
		}
		else {
			filename[i][strlen(filename[i]) -1] = 'o';
			strcat(buffer, filename[i]);
		}
		strcat(buffer, " -O3 -lGL -lGLU -lGLEW -lglut -lddcglut -lm -w -lSDL2 -lSDL2_mixer");
		if (includePath) {
			strcat(buffer, " -I ");
			strcat(buffer, includePath);
		}
		//printf("command: %s\n", buffer);
		system(buffer);
	}

	if (1) {
		//printf("~~~ asset phase ~~~\n");
	}
	// compile all given input files
	for (int i = 0; i < input_file_total && compile; i++) {

		// check if file is asset
		if (strcmp(filename[i] +strlen(filename[i]) -4, ".ply") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".bmp") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") != 0) {
			continue;
		}

		if (outname) {
			strcpy(buffer, outname);
		}
		else {
			strcpy(buffer, filename[i]);
			buffer[strlen(buffer) -4] = '\0';
			strcat(buffer, ".asset");
		}

		//printf("compiling asset: %s to %s\n", filename[i], buffer);

		file_copy(filename[i], buffer, 0);
	}

	// compile the final executable
	if (link) {
		//printf("~~~ link phase ~~~\n");
		buffer[0] = '\0';
		sprintf(buffer, "gcc ");
		for (int i = 0; i < input_file_total; i++) {
			strcat(buffer, filename[i]);
			strcat(buffer, " ");
		}
		strcat(buffer, "-O3 -lGL -lGLU -lGLEW -lglut -lddcglut -lm -w -lSDL2 -lSDL2_mixer -o ");
		if (outname) {
			strcat(buffer, outname);
		}
		else {
			strcat(buffer, "game");
		}
		//printf("command: %s\n", buffer);
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
		 * Attempt to parse native command
		 * if no ast_node is returned, it must be a custom command
		 * Custom commands are currently "Pass-Through"
		 *
		 */
		struct ast_node *cmd = parse_command(cmd_name, opt_args);

		// Known (native) command
		if (cmd) {
			ast_push(cmd);
		}
		// Not a known command, must be a custom one
		else {
			struct entry *e = symtable_entryat(cmd_name->value);
			sprintf(buffer, "not a keyword: '%s'", e->lexptr);
			//yyerror(buffer);
			//printf("error symbol not keyword: %s\n", e->lexptr);

			/* command node
			 * construct it in such a way that the parent ast node is the command
			 * if a custom command, then first child is its name
			 */
			opt_args->node_type = AST_COMMAND_CUSTOM;
			opt_args->value = $2;
			ast_child_add_first(opt_args, cmd_name);
			opt_args->isIncluded = include_stack_ptr != 0;
			ast_push(opt_args);
		}
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
