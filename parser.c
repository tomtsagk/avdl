#include "parser.h"
#include "stdlib.h"
#include "stdio.h"
#include "yacc.tab.h"
#include "symtable.h"
#include "structtable.h"
#include <string.h>

void print_node(FILE *fd, struct ast_node *n);

// file descriptor for global data
FILE *fd_global;

void print_node(FILE *fd, struct ast_node *n) {

}

// responsible for creating a file and translating ast to target language
void parse(const char *filename, struct ast_node *n) {
	fd_global = fopen(filename, "w");
	fprintf(fd_global, "#include <stdlib.h>\n");
	print_node(fd_global, n);
	fclose(fd_global);
}
