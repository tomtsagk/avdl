#ifndef _PGMPREFIX_AST_NODE_H_
#define _PGMPREFIX_AST_NODE_H_

#include "dd_dynamic_array.h"

/* abstract syntax tree
 * contains nodes that represent logic inside the game
 * each node has a unique "signature" (amount of children) to be considered
 * correct, and be able to parse itself properly to the target language
 */

// ast node types
enum AST_NODE_TYPE {

	/* parent of all nodes
	 * children: statements
	 */
	AST_GAME,

	AST_EMPTY,

	AST_COMMAND,
	AST_GROUP,
	AST_FUNCTION,

	/* variables - contain children that go through the struct hierarchy
	 * example "this.x" results in a node AST_IDENTIFIER that points to the struct of "this" and
	 * a child with another AST_IDENTIFIER that points to "x"
	 */
	AST_IDENTIFIER,

	/* constants - have no children, are parsed as-is */
	AST_NUMBER,
	AST_STRING,
};

// Struct for a single node
struct ast_node {
	enum AST_NODE_TYPE node_type;
	int value;
	struct dd_dynamic_array children;
};

// Actions
struct ast_node *ast_create(enum AST_NODE_TYPE node_type, int value);
void ast_child_add(struct ast_node *parent, struct ast_node *child);
void ast_child_add_first(struct ast_node *parent, struct ast_node *child);
void ast_delete(struct ast_node *node);

// Debug - Print node tree
void ast_print(struct ast_node *node);

int ast_push(struct ast_node *n);
struct ast_node *ast_pop();

#endif
