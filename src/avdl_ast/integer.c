#include "avdl_ast/integer.h"
#include "avdl_ast_node.h"
#include "avdl_log.h"

struct ast_node *avdl_ast_integer_Create(int value) {
	struct ast_node *node = ast_create(AST_NUMBER);
	ast_setValuei(node, value);
	return node;
}

int avdl_ast_integer_PrintToC(struct ast_node *o, FILE *fd) {
	fprintf(fd, "%d", o->value);
	return 0;
}

int avdl_ast_integer_IsValid(struct ast_node *o) {
	return o->node_type == AST_NUMBER;
}
