#ifndef AVDL_SEMANTIC_ANALYSER
#define AVDL_SEMANTIC_ANALYSER

#include "avdl_ast_node.h"

int semanticAnalyser_convertToAst(struct ast_node *node, const char *filename);

#endif
