#include "avdl_ast_node.h"
#include <assert.h>

int main(int argc, char *argv[]) {

	// check the creation of new nodes
	struct ast_node *my_node = 0;
	my_node = ast_create(AST_GAME, 0);
	assert(my_node != 0);

	// check getting its type
	assert(ast_get_type(my_node) == AST_GAME);

	// check adding children to nodes
	ast_child_add(my_node, ast_create(AST_GROUP, 0));
	assert(ast_get_childCount(my_node) == 1);

	struct ast_node *child_node = ast_get_child(my_node, 0);
	assert(child_node != 0);
	assert(ast_get_type(child_node) == AST_GROUP);

	return 0;
}
