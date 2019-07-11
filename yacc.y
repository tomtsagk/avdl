%{
#include <stdio.h>
#include <string.h>
#include "yacc.tab.h"
#include "symtable.h"
#include "ast_node.h"
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
 
// line number (got from lex.l)
extern int linenum;

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
};

// init data, parse, exit
int main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf("arg 1 should be output file\n");
		return -1;
	}

	// init data
	linenum = 1;

	// initial symbols
	symtable_init();

	/* keywords
	 */
	for (int i = 0; i < sizeof(keywords) /sizeof(char*); i++) {
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
	parse_javascript(argv[1], game_node);

	//struct_print();

	// print debug data and clean everything
	/*
	symtable_print();
	symtable_clean();

	*/
	ast_print(game_node);
	ast_delete(game_node);

	// success!
	return 0;
} 

%}

%token DD_ZERO

%token DD_KEYWORD

/* constants */
%token DD_CONSTANT_SYMBOL DD_CONSTANT_STRING DD_CONSTANT_NUMBER

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
	'(' DD_CONSTANT_SYMBOL optional_args ')' {
		struct entry *e = symtable_entryat($2);
		if (e->token == DD_KEYWORD) {
    			//printf("keyword symbol: %s\n", e->lexptr);
		}
		else {
			sprintf(buffer, "not a keyword: '%s'", e->lexptr);
			yyerror(buffer);
	    		//printf("error symbol not keyword: %s\n", e->lexptr);
		}

		/* command node
		 */
		struct ast_node *n = ast_create(AST_COMMAND, 0);

		struct ast_node *opt_args = ast_pop();
		ast_child_add(n, opt_args);

		ast_push(n);
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
   	DD_CONSTANT_NUMBER {
		struct ast_node *n = ast_create(AST_NUMBER, $1);
		ast_push(n);
	}
	|
   	DD_CONSTANT_STRING {
		struct entry *e = symtable_entryat($1);
		struct ast_node *n = ast_create(AST_STRING, $1);
		ast_push(n);
	}
	|
	DD_CONSTANT_SYMBOL {
		struct entry *e = symtable_entryat($1);
		struct ast_node *n = ast_create(AST_IDENTIFIER, $1);
		ast_push(n);
	}
	|
	command
	;
