#ifndef AVDL_AST_NODE_H
#define AVDL_AST_NODE_H

#include "avdl_dynamic_array.h"

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

	AST_GROUP,
	AST_COMMAND_NATIVE,
	AST_COMMAND_CUSTOM,

	/* variables - contain children that go through the struct hierarchy
	 * example "this.x" results in a node AST_IDENTIFIER that points to the struct of "this" and
	 * a child with another AST_IDENTIFIER that points to "x"
	 */
	AST_IDENTIFIER,

	/* constants - have no children, are parsed as-is */
	AST_NUMBER,
	AST_FLOAT,
	AST_STRING,

	AST_INCLUDE,
};

// Struct for a single node
struct ast_node {

	// type and its value (int or float)
	enum AST_NODE_TYPE node_type;
	union {
		int value;
		float fvalue;
	};

	// space to add string, useful for variables
	char lex[500];

	// values that may or may not stay here
	int arraySize;
	int isRef;
	int isExtern;
	int isIncluded;

	// node may have a parent, and any number of children
	struct dd_dynamic_array children;
	struct ast_node *parent;
};

// lifecycle
struct ast_node *ast_create(enum AST_NODE_TYPE node_type);
int ast_addChild  (struct ast_node *parent, struct ast_node *child);
int ast_addChildAt(struct ast_node *parent, struct ast_node *child, int index);
int ast_delete(struct ast_node *n);

// setters
int ast_setValuei(struct ast_node *n, int value);
int ast_setValuef(struct ast_node *n, float value);
int ast_setLex   (struct ast_node *n, const char *newLex);

// getters
int   ast_getChildCount(struct ast_node *n);
int   ast_getValuei    (struct ast_node *n);
float ast_getValuef    (struct ast_node *n);
const char *ast_getLex (struct ast_node *n);
enum AST_NODE_TYPE ast_getType(struct ast_node *n);
struct ast_node *ast_getChild (struct ast_node *n, int index);
struct ast_node *ast_getParent(struct ast_node *n);

// Debug - Print node tree
void ast_print(struct ast_node *n);

#endif
