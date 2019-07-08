#ifndef _LANG_PARSER_H_
#define _LANG_PARSER_H_

#include "ast_node.h"

// parse ast_node tree
void parse_javascript(const char *filename, struct ast_node *n);

#endif
