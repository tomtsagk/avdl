#ifndef AVDL_LEXER_H
#define AVDL_LEXER_H

#include "ast_node.h"

void lexer_convertToAst(struct ast_node *node, const char *filename);

#endif
