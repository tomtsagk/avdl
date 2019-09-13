%{
#include <stdio.h>
#include <string.h>
#include "yacc.tab.h"
#include "symtable.h"
#include "ast_node.h"
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>

extern FILE *yyin;
extern YYSTYPE yylex(void);

// line number (got from lex.l)
extern int linenum;

extern float parsing_float;

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

char *keywords[] = {
	"echo",
	"def",
	"=",
	"+",
	"-",
	"*",
	"/",
	">=",
	"==",
	"<=",
	"&&",
	"||",
	"<",
	">",
	"group",
	"class",
	"function",
	"return",
	"array",
	"new",
	"if",
};

// init data, parse, exit
int main(int argc, char *argv[])
{
	/* tweakable data
	 */
	int show_ast = 0;
	char *filename = 0;
	FILE *input_file = stdin;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// print abstract syntax tree
		if (strcmp(argv[i], "--print-ast") == 0) {
			show_ast = 1;
		}
		else
		// input file
		if (!filename) {
			filename = argv[i];
		}
	}

	// make sure the minimum to parse exists
	if (!filename) {
		printf("no filename given\n");
		return -1;
	}
	else {
		input_file = fopen(filename, "r");
		if (!input_file) {
			printf("unable to open input file: %s\n", filename);
			return -1;
		}

		yyin = input_file;
	}

	// init data
	linenum = 1;

	// initial symbols
	symtable_init();

	/* keywords
	 */
	for (unsigned int i = 0; i < sizeof(keywords) /sizeof(char*); i++) {
		symtable_insert(keywords[i], DD_KEYWORD);
	}
	/*
	symtable_insert("DD_WIDTH", DD_INTERNAL_WIDTH);
	symtable_insert("DD_HEIGHT", DD_INTERNAL_HEIGHT);
	*/

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

	// parse!
        yyparse();

	// parse resulting ast tree to a file
	parse_javascript("build/game.js", game_node);

	//struct_print();

	// print debug data and clean everything
	/*
	symtable_print();
	*/

	if (show_ast) {
		ast_print(game_node);
	}

	// clean symtable and ast tree
	symtable_clean();
	ast_delete(game_node);

	// success!
	return 0;
}

%}

// nothingness
%token DD_ZERO

// used for the languages keywords
%token DD_KEYWORD

/* constants */
%token DD_CONSTANT_SYMBOL DD_CONSTANT_STRING DD_CONSTANT_NUMBER DD_CONSTANT_FLOAT

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
	}
	;

/* single command
 * has a keyword and optional arguments
 */
command:
	'(' cmd_name optional_args ')' {

		// get nodes
		struct ast_node *opt_args = ast_pop();
		struct ast_node *cmd_name = ast_pop();

		// find out if keyword is an native keyword or a custom one
		struct entry *e = symtable_entryat(cmd_name->value);
		int type;

		if (e->token == DD_KEYWORD) {
    			//printf("keyword symbol: %s\n", e->lexptr);
			if (strcmp(e->lexptr, "group") == 0) {
				type = AST_GROUP;
			}
			else {
				type = AST_COMMAND_NATIVE;
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
		ast_push(opt_args);
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
	DD_CONSTANT_SYMBOL {
		struct ast_node *n = ast_create(AST_IDENTIFIER, $1);
		ast_push(n);
	}
	|
	identifier '.' DD_CONSTANT_SYMBOL {
		struct ast_node *new = ast_create(AST_IDENTIFIER, $3);
		struct ast_node *group = ast_pop();
		ast_child_add(group, new);
		ast_push(group);
	}
	;
