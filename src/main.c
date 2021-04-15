#include <stdio.h>
#include <string.h>
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
#include "lexer.h"
#include "semantic_analyser.h"

/*
extern FILE *yyin;
extern YYSTYPE yylex(void);
*/

// line number (got from lex.l)
//extern int linenum;

extern float parsing_float;

//extern int include_stack_ptr;

char included_files[10][100];
int included_files_num = 0;

// buffer for general use
#define DD_BUFFER_SIZE 1000
char buffer[DD_BUFFER_SIZE];

/*
// error
void yyerror(const char *str)
{
        fprintf(stderr,"error: line %d: %s\n", linenum, str);
	_exit(-1);
}
*/

// game node, parent of all nodes
struct ast_node *game_node;

#define MAX_INPUT_FILES 10

enum AVDL_PLATFORM avdl_platform = AVDL_PLATFORM_NATIVE;

const char *dependencies[] = {
	"GLEW",
	"m",
	"glut",
	"SDL2-2.0",
	"SDL2_mixer-2.0",
};
unsigned int dependencies_count = sizeof(dependencies) /sizeof(char *);

char *includePath = 0;

// init data, parse, exit
int main(int argc, char *argv[])
{
	/* tweakable data
	 */
	int show_ast = 0;
	int show_struct_table = 0;
	char filename[MAX_INPUT_FILES][100];
	int input_file_total = 0;
	char *outname = 0;
	int getDependencies = 0;

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

		if (strcmp(argv[i], "--dependencies") == 0) {
			getDependencies = 1;
		}
		else
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
		 * -t: translate only
		 * -c: skip linking
		 */
		if (strcmp(argv[i], "-t") == 0) {
			compile = 0;
			link = 0;
		}
		else
		if (strcmp(argv[i], "-c") == 0) {
			link = 0;
		}
		else
		// output filename
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
		// add include path
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
		// compiling for android
		if (strcmp(argv[i], "--android") == 0) {
			avdl_platform = AVDL_PLATFORM_ANDROID;
			link = 0;
		}
		else
		// input file
		if (input_file_total < MAX_INPUT_FILES) {
			strncpy(filename[input_file_total], argv[i], 100);
			filename[input_file_total][99] = '\0';
			input_file_total++;
		}
		// error argument
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

	if (getDependencies) {
		for (unsigned int i = 0; i < dependencies_count; i++) {
			strcpy(buffer, "ldd ");
			strcat(buffer, filename[0]);
			strcat(buffer, " | grep 'lib");
			strcat(buffer, dependencies[i]);
			strcat(buffer, "\\.' | sed 's/^.*=> \\(.*\\) .*$/\\1/'");
			//printf("command: %s\n", buffer);
			system(buffer);
		}
		return 0;
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
		//include_stack_ptr = 0;

		/*
		FILE *input_file = 0;
		input_file = fopen(filename[i], "r");
		if (!input_file) {
			printf("avdl error: Unable to open '%s': %s\n", filename[i], strerror(errno));
			return -1;
		}
		*/

		//yyin = input_file;

		// init data
		//linenum = 1;

		// initial symbols
		//symtable_init();

		// initialise the parent node

		// init structs
		struct_table_init();

		game_node = ast_create(AST_GAME, 0);
		semanticAnalyser_convertToAst(game_node, filename[i]);
		//lexer_convertToAst(game_node, filename[i]);
		//ast_print(game_node);
		//dd_commands_validate(game_node);

		//yyparse();
		//fclose(input_file);

		/*
		 * if only transpiling, check output file
		 */
		//printf("transpiling: %s", filename[i]);
		// parse resulting ast tree to a file
		if (!compile && !link && outname) {
			strcpy(buffer, outname);
		}
		else {
			filename[i][strlen(filename[i]) -2] = 'c';
			filename[i][strlen(filename[i]) -1] = '\0';
			strcpy(buffer, filename[i]);
		}
		//printf(" to %s\n", buffer);
		//if (transpile_cglut(buffer, game_node, 0) != 0) {
		if (transpile_cglut2(buffer, game_node) != 0) {
			printf("avdl: transpilation failed to file: %s\n", buffer);
			return -1;
		}

		// print debug data
		if (show_ast) {
			ast_print(game_node);
		}

		if (show_struct_table) {
			struct_table_print();
		}

		//symtable_print();

		// clean symtable and ast tree
		//symtable_clean();
		//ast_delete(game_node);

//		for (int i = 0; i < included_files_num; i++) {
//			//printf("transpiling included file: %s\n", included_files[i]);
//			include_stack_ptr = 0;
//			symtable_clean();
//			/*
//			//input_file = fopen(included_files[i], "r");
//			if (!input_file) {
//				printf("avdl error: Unable to open included file '%s': %s\n", included_files[i], strerror(errno));
//				return -1;
//			}
//			*/
//
//			strcpy(buffer, included_files[i]);
//			buffer[strlen(buffer)-3] = 'h';
//			buffer[strlen(buffer)-2] = '\0';
//			//printf("transpiling to: %s\n", buffer);
//			//sprintf(buffer, "build-cglut/%s.h", included_files[i]);
//
//			/*
//			// stat source and destination asset, if source is newer, copy to destination
//			struct stat stat_src_asset;
//			if (stat(included_files[i], &stat_src_asset) == -1) {
//				printf("avdl error: Unable to stat '%s': %s\n", included_files[i], strerror(errno));
//				exit(-1);
//			}
//			*/
//
//			//struct stat stat_dst_asset;
//			//if (stat(buffer, &stat_dst_asset) == -1) {
//			if (1) {
//				//yyin = input_file;
//				game_node = ast_create(AST_GAME, 0);
//				yyparse();
//				//fclose(input_file);
//				if (transpile_cglut(buffer, game_node, 1) != 0) {
//					printf("avdl: transpilation failed to file: %s\n", buffer);
//					return -1;
//				}
//				if (show_ast) {
//					ast_print(game_node);
//				}
//				continue;
//			}
//
//			/*
//			// check last modification time
//			time_t time_src = mktime(gmtime(&stat_src_asset.st_mtime));
//			time_t time_dst = mktime(gmtime(&stat_dst_asset.st_mtime));
//			*/
//
//			//if (time_src > time_dst) {
//			if (1) {
//				//yyin = input_file;
//				game_node = ast_create(AST_GAME, 0);
//				yyparse();
//				//fclose(input_file);
//				if (transpile_cglut(buffer, game_node, 1) != 0) {
//					printf("avdl: transpilation failed to file: %s\n", buffer);
//					return -1;
//				}
//				if (show_ast) {
//					ast_print(game_node);
//				}
//				continue;
//			}
//		}
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
		strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -c -w ");
		strcat(buffer, filename[i]);
		strcat(buffer, " -o ");
		if (outname && !link) {
			strcat(buffer, outname);
		}
		else {
			filename[i][strlen(filename[i]) -1] = 'o';
			strcat(buffer, filename[i]);
		}
		strcat(buffer, " -O3 -lGL -lGLU -lGLEW -lglut -lavdl-cengine -lm -w -lSDL2 -lSDL2_mixer");
		if (includePath) {
			strcat(buffer, " -I ");
			strcat(buffer, includePath);
		}
		//printf("command: %s\n", buffer);
		if (system(buffer)) {
			printf("avdl: error compiling file: %s\n", filename[i]);
			exit(-1);
		}
	}

	if (1) {
		//printf("~~~ asset phase ~~~\n");
	}
	// compile all given input files
	for (int i = 0; i < input_file_total && compile; i++) {

		// check if file is asset
		if (strcmp(filename[i] +strlen(filename[i]) -4, ".ply") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".bmp") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".ogg") != 0) {
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
		sprintf(buffer, "gcc -DDD_PLATFORM_NATIVE ");
		for (int i = 0; i < input_file_total; i++) {
			strcat(buffer, filename[i]);
			strcat(buffer, " ");
		}
		strcat(buffer, "-O3 -lGL -lGLU -lGLEW -lglut -lavdl-cengine -lm -w -lSDL2 -lSDL2_mixer -lpthread -o ");
		if (outname) {
			strcat(buffer, outname);
		}
		else {
			strcat(buffer, "game");
		}
		//printf("command: %s\n", buffer);
		if (system(buffer)) {
			printf("avdl: error linking files\n");
			exit(-1);
		}
	}

	// success!
	return 0;
}
