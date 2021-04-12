#ifndef AVDL_SEMANTIC_ANALYSER
#define AVDL_SEMANTIC_ANALYSER

#include "ast_node.h"

void semanticAnalyser_convertToAst(struct ast_node *node, const char *filename);

#endif
