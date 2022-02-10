#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "avdl_ast_node.h"

int main(int argc, char *argv[]) {

	// create ast
	struct ast_node *n = ast_create(AST_GAME);
	assert(n);
	assert(ast_getChildCount(n) == 0);
	assert(ast_getType(n) == AST_GAME);

	// add children
	struct ast_node *c1 = ast_create(AST_NUMBER);
	assert(c1);
	assert(ast_addChild(n, c1));
	assert(ast_getChildCount(n) == 1);

	struct ast_node *c2 = ast_create(AST_FLOAT);
	assert(c2);
	assert(ast_addChild(n, c2));
	assert(ast_getChildCount(n) == 2);

	struct ast_node *c3 = ast_create(AST_IDENTIFIER);
	assert(c3);

	struct ast_node *c4 = ast_create(AST_GROUP);
	assert(c4);
	assert(ast_addChild(c3, c4));
	assert(ast_getChildCount(c3) == 1);

	assert(ast_addChildAt(n, c3, 0));
	assert(ast_getChildCount(n) == 3);

	// confirm children
	struct ast_node *rc1 = ast_getChild(n, 0);
	struct ast_node *rc2 = ast_getChild(n, 1);
	struct ast_node *rc3 = ast_getChild(n, 2);

	assert(rc1);
	assert(rc2);
	assert(rc3);

	assert(ast_getType(rc1) == AST_IDENTIFIER);
	assert(ast_getType(rc2) == AST_NUMBER);
	assert(ast_getType(rc3) == AST_FLOAT);

	assert(ast_getParent(rc1) == n);
	assert(ast_getParent(rc2) == n);
	assert(ast_getParent(rc3) == n);

	// change values - ints
	assert(ast_setValuei(n, 5));
	assert(ast_getValuei(n) == 5);
	assert(ast_setValuei(n, 8));
	assert(ast_getValuei(n) == 8);

	// change values - floats for now guarantee 5 digits accuracy
	assert(ast_setValuef(n, 1.3));
	assert(fabs(ast_getValuef(n) - 1.3) < 0.000001);
	assert(ast_setValuef(n, 1.23456));
	assert(fabs(ast_getValuef(n) - 1.23456) < 0.000001);

	// change values - lex
	assert(ast_setLex(n, "my string"));
	assert(strcmp(ast_getLex(n), "my string") == 0);
	assert(ast_setLex(n, "another string"));
	assert(strcmp(ast_getLex(n), "another string") == 0);

	// confirm errors
	assert(!ast_setValuei(0, 0));
	assert(!ast_setValuef(0, 0));
	assert(!ast_setLex(0, 0));

	struct ast_node *c5 = ast_create(AST_EMPTY);
	assert(!ast_addChildAt(0, c5, 0));
	assert(!ast_addChildAt(n, 0, 0));
	assert(!ast_addChildAt(n, c5, 10));
	assert(!ast_delete(0));
	assert(!ast_getChildCount(0));
	assert(!ast_getChild(0, 0));
	assert(!ast_getChild(n, -1));
	assert(!ast_getChild(n, 100));
	assert(ast_getType(0) == AST_EMPTY);
	assert(!ast_getValuei(0));
	assert(!ast_getValuef(0));
	assert(ast_delete(c5));

	char bigArray[600];
	memset(bigArray, 'a', 600);
	bigArray[599] = '\0';
	assert(!ast_setLex(n, bigArray));
	assert(!ast_getLex(0));
	assert(!ast_getParent(0));

	// clean memory
	assert(ast_delete(n));

	// recycle object
	n = ast_create(AST_GAME);
	assert(n);
	assert(ast_delete(n));

	// all pass!
	return 0;
}
