#ifndef _LANG_PARSER_H_
#define _LANG_PARSER_H_

#include "ast_node.h"

// transpile ast_node tree to file
int transpile_cglut(const char *filename, struct ast_node *n, int isIncluded);

#endif
