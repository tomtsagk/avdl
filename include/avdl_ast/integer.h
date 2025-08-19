#include "avdl_ast_node.h"
#include "avdl_lexer.h"
#include <stdio.h>

struct ast_node *avdl_ast_integer_Expect(struct avdl_lexer *l);
struct ast_node *avdl_ast_integer_Create(int value);

int avdl_ast_integer_PrintToC(struct ast_node *o, FILE *fd);
int avdl_ast_integer_IsValid(struct ast_node *o);

int avdl_ast_integer_GetValue(struct ast_node *o);
