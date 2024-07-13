#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "avdl_symtable.h"
#include "avdl_ast_node.h"
#include "avdl_dynamic_array.h"
#include "avdl_std.h"

// Create node with given token and value - no children
struct ast_node *ast_create(enum AST_NODE_TYPE node_type) {

	// initialise the node itself
	struct ast_node *new_node = avdl_malloc(sizeof(struct ast_node));
	if (!new_node) {
		return 0;
	}

	// set its type and default value
	new_node->node_type = node_type;
	ast_setValuei(new_node, 0);

	// lex for variables
	new_node->lex[0] = '\0';

	// data that may or may not remain here
	new_node->arraySize = -1;
	new_node->isRef = 0;
	new_node->isIncluded = 0;
	new_node->isStruct = 0;

	// initialise children and parents
	avdl_da_init(&new_node->children, sizeof(struct ast_node));
	/* avdl_da_init cannot fail for now
	if (!avdl_da_init(&new_node->children, sizeof(struct ast_node))) {
		avdl_free(new_node);
		return 0;
	}
	*/
	new_node->parent = 0;

	// return newly created node
	return new_node;

}

// add child at end of array
int ast_addChild(struct ast_node *parent, struct ast_node *child) {
	return ast_addChildAt(parent, child, ast_getChildCount(parent));
}

// add child at specific spot
int ast_addChildAt(struct ast_node *parent, struct ast_node *child, int index) {

	// reject wrong values
	if (!parent || !child
	|| index < 0
	|| index > ast_getChildCount(parent)) {
		return 0;
	}

	// attempt to add child to array
	child->parent = parent;
	if (!avdl_da_add(&parent->children, child, 1, index)) {
		child->parent = 0;
		return 0;
	}

	// the original child is destroyed
	avdl_free(child);

	// success
	return 1;
}

// delete children and children of children
static void ast_delete_children(struct ast_node *n) {
	for (unsigned int i = 0; i < ast_getChildCount(n); i++) {
		ast_delete_children(ast_getChild(n, i));
	}
	avdl_da_free(&n->children);
}

// deletes children and the node itself
int ast_delete(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	ast_delete_children(n);
	avdl_free(n);
	return 1;
}

// setter for integer value
int ast_setValuei(struct ast_node *n, int value) {
	if (!n) {
		return 0;
	}
	n->value = value;
	return 1;
}

// setter for float value
int ast_setValuef(struct ast_node *n, float value) {
	if (!n) {
		return 0;
	}
	n->fvalue = value;
	return 1;
}

// adds a word, currently with a limit of 500 characters
int ast_setLex(struct ast_node *n, const char *newLex) {
	if (!n || !newLex || strlen(newLex) > 499) {
		return 0;
	}
	strcpy(n->lex, newLex);
	n->lex[499] = '\0';
	return 1;
}

// getters
int ast_getChildCount(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	return avdl_da_count(&n->children);
}

struct ast_node *ast_getChild(struct ast_node *n, int index) {
	if (!n || index < 0 || index > ast_getChildCount(n) -1) {
		return 0;
	}
	return avdl_da_get(&n->children, index);
}

enum AST_NODE_TYPE ast_getType(struct ast_node *n) {
	if (!n) {
		return AST_EMPTY;
	}
	return n->node_type;
}

int ast_getValuei(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	return n->value;
}

float ast_getValuef(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	return n->fvalue;
}

const char *ast_getLex(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	return n->lex;
}

struct ast_node *ast_getParent(struct ast_node *n) {
	if (!n) {
		return 0;
	}
	return n->parent;
}

/*
 * Print whole node tree, meant for debugging only
 * might delete, or modify at a later point
int tabs = 0;
void ast_print(struct ast_node *node) {

	// Print tabs (if any)
	for (int i = 0; i < tabs; i++) {
		printf("\t");
	}

	if (tabs == 0) {
		printf("Abstract Syntax Tree:\n");
		printf("*** ");
	}
	else {
		printf("* ");
	}

	// Print actual node
	switch (node->node_type) {

		case AST_GAME: printf("GAME"); break;
		case AST_NUMBER: printf("NUMBER: %d", node->value); break;
		case AST_FLOAT: printf("FLOAT: %f", node->fvalue); break;
		case AST_STRING:
			printf("STRING: \"%s\"", node->lex);
			break;
		case AST_GROUP: printf("GROUP"); break;
		case AST_COMMAND_NATIVE: printf("COMMAND NATIVE: %s", node->lex); break;
		case AST_COMMAND_CUSTOM: printf("COMMAND CUSTOM: %s", node->lex); break;
		case AST_INCLUDE: printf("INCLUDE:"); break;

		case AST_IDENTIFIER:
			printf("IDENTIFIER: %s", node->lex);
			break;

		default:
			printf("%d | %d", node->node_type, node->value);
			break;
	}

	printf("\n");

	// Print children
	tabs++;
	for (unsigned int i = 0; i < node->children.elements; i++) {
		struct ast_node *child = avdl_da_get(&node->children, i);
		ast_print(child);
	}
	tabs--;
}
 */
