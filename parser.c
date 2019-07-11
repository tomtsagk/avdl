#include "parser.h"
#include "stdlib.h"
#include "stdio.h"
#include "yacc.tab.h"
#include "symtable.h"
#include <string.h>

#define DD_BUFFER_SIZE 1000
static char buffer[DD_BUFFER_SIZE];

void print_node(FILE *fd, struct ast_node *n);

// file descriptor for global data
FILE *fd_global;

struct command_translation {
	char *command;
	char *translation;
};

static struct command_translation translations[] = {
	{"echo", "console.log(~n{+});\n"},
	{"int", "var ~0;\n"},
};

void print_node(FILE *fd, struct ast_node *n) {
	switch (n->node_type) {
		case AST_GAME:
		case AST_GROUP:
			for (int i = 0; i < n->children.elements; i++) {
				print_node(fd, dd_da_get(&n->children, i));
			}
			break;
		case AST_COMMAND: {
			struct entry *e = symtable_entryat(n->value);

			for (int i = 0; i < sizeof(translations) /sizeof(struct command_translation); i++) {

				if (strcmp(e->lexptr, translations[i].command) == 0) {

					char *translation = translations[i].translation;

					buffer[0] = '\0';
					char *sub;
					do {
						// found argument
						sub = strchr(translation, '~');
						if (sub) {

							// add everything so far to the buffer
							strncat(buffer, translation, sub -translation);
							fprintf(fd, buffer);
							buffer[0] = '\0';

							// parse argument
							sub++;

							// print everything
							if (sub[0] == 'n') {
								sub++;
								int start = 0;
								int end = n->children.elements;
								if (sscanf(sub, "%d-%d")) {
									start = atoi(sub);
									while(sub[0] >= '0' && sub[0] <= '9') sub++;
									sub++;
									end = atoi(sub);
									while(sub[0] >= '0' && sub[0] <= '9') sub++;
								}

								char between = ' ';
								if (sscanf(sub, "{%c}", &between)) {
									sub = sub +3;
								}

								for (int i = start; i < end; i++) {
									if (i != start) {
										fprintf(fd, "%c", between);
									}
									print_node(fd, dd_da_get(&n->children, i));
								}

							}
							// single number argument
							else {

								int arg = atoi(sub);

								// remove argument from string
								while(sub[0] >= '0' && sub[0] <= '9') sub++;

								// call argument, to fill in the blanks
								print_node(fd, dd_da_get(&n->children, arg));
							}

							translation = sub;
						}
					} while (sub);
					fprintf(fd, translation);
					break;
				}
			}
			break;
		}
		case AST_NUMBER: {
			fprintf(fd, "%d", n->value);
			break;
		}
		case AST_STRING: {
			struct entry *e = symtable_entryat(n->value);
			fprintf(fd, "'%s'", e->lexptr);
			break;
		}
		case AST_IDENTIFIER: {
			struct entry *e = symtable_entryat(n->value);
			fprintf(fd, "%s", e->lexptr);
			break;
		}
	}
}

// responsible for creating a file and translating ast to target language
void parse_javascript(const char *filename, struct ast_node *n) {
	fd_global = fopen(filename, "w");
	print_node(fd_global, n);
	fclose(fd_global);
}
