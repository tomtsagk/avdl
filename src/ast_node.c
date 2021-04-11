#include "ast_node.h"
#include <stdlib.h>
#include <stdio.h>
#include "symtable.h"
#include <string.h>

// Create node with given token and value - no children
struct ast_node *ast_create(enum AST_NODE_TYPE node_type, int value) {

	struct ast_node *new_node = malloc(sizeof(struct ast_node));
	new_node->node_type = node_type;
	new_node->value = value;
	new_node->arraySize = -1;
	new_node->isRef = 0;
	new_node->isIncluded = 0;
	new_node->lex[0] = '\0';
	dd_da_init(&new_node->children, sizeof(struct ast_node));
	new_node->parent = 0;
	return new_node;

}

void ast_child_add(struct ast_node *parent, struct ast_node *child) {
	dd_da_add(&parent->children, child);
	free(child);
}

void ast_child_add_first(struct ast_node *parent, struct ast_node *child) {
	dd_da_add_first(&parent->children, child);
	free(child);
}

void ast_delete_child(struct ast_node *node) {

	// Delete children
	for (unsigned int i = 0; i < node->children.elements; i++) {
		struct ast_node *child = dd_da_get(&node->children, i);
		ast_delete_child(child);
	}

	dd_da_free(&node->children);
}

void ast_delete(struct ast_node *node) {
	ast_delete_child(node);
	free(node);
}

// Print whole node tree, meant for debugging only
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

	if (node->isIncluded) {
		printf(" included: %d", node->isIncluded);
	}

	printf("\n");

	// Print children
	tabs++;
	for (unsigned int i = 0; i < node->children.elements; i++) {
		struct ast_node *child = dd_da_get(&node->children, i);
		ast_print(child);
	}
	tabs--;
}

// AST table
#define AST_TABLE_MAX 1000
struct ast_node* ast_table[AST_TABLE_MAX];
int ast_table_lastentry;

int ast_push(struct ast_node *n) {
	if (ast_table_lastentry +1 >= AST_TABLE_MAX) {
		return 0;
	}

	ast_table_lastentry++;
	ast_table[ast_table_lastentry] = n;

	// everything is ok, return a true value
	return 1;
}

struct ast_node *ast_pop() {
	if (ast_table_lastentry < 0) {
		return 0;
	}
	ast_table_lastentry--;
	return ast_table[ast_table_lastentry+1];
}

void ast_addLex(struct ast_node *n, const char *newLex) {
	if (strlen(newLex) > 499) {
		printf("lex is too long: %s\n", newLex);
		exit(-1);
	}

	strcpy(n->lex, newLex);
	n->lex[499] = '\0';
}
