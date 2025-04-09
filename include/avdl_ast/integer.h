#include "avdl_ast_node.h"
#include <stdio.h>

struct ast_node *avdl_ast_integer_Create(int value);
int avdl_ast_integer_PrintToC(struct ast_node *o, FILE *fd);
int avdl_ast_integer_IsValid(struct ast_node *o);
