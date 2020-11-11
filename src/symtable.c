#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"
#include "yacc.tab.h"

int symtable_current = -1;

static const char *native_functions[] = {
	"dd_translatef",
	"dd_math_rand",
	"dd_math_ease_linear",
	"dd_math_ease_linear2d",
	"dd_math_ease_bezier",
	"dd_math_ease_bezier2d",
	"dd_math_ease_catmullrom",
	"dd_math_ease_catmullrom2d",
	"dd_math_pow",
	"dd_math_abs",
	"dd_math_sqrt",
	"dd_math_sin",
	"dd_math_cos",
	"dd_math_tan",
	"dd_math_asin",
	"dd_math_acos",
	"dd_math_atan",
	"dd_math_min",
	"dd_math_max",
	"dd_math_minf",
	"dd_math_maxf",
	"dd_math_tan",
	"dd_math_rad2dec",
	"dd_math_dec2rad",
	"dd_math_dot2",
	"dd_math_dot3",
};
static unsigned int native_functions_count = sizeof(native_functions) /sizeof(char *);

// number of entries on each symtable
#define SYMMAX 600

// how many scopes to descent
#define SYMTABLE_SCOPE_MAX 100

// symtable
struct symtable {
	struct entry entry[SYMMAX];
	int lastentry;
} symtable_array[SYMTABLE_SCOPE_MAX], *symtable;

/* init
 * allocate memory
 * no symbols inside
 */
void symtable_init() {
	symtable_current = -1;
	symtable_push();

	// init language's native functions
	for (unsigned int i = 0; i < native_functions_count; i++) {
		symtable_insert(native_functions[i], DD_FUNCTION);
	}

}

void symtable_push() {
	if (symtable_current == SYMTABLE_SCOPE_MAX-1) {
		printf("symtable_push: cannot push another table\n");
		exit(-1);
	}
	symtable_current++;
	symtable = &symtable_array[symtable_current];
	symtable->lastentry = -1;
}

void symtable_pop() {
	if (symtable_current == -1) {
		printf("symtable_pop: no table to pop\n");
		exit(-1);
	}
	symtable_current--;
	symtable = &symtable_array[symtable_current];
}

// find symbol, and return its index
symtable_index symtable_lookup(const char s[]) {
	int p;
	for (p = 0; p <= symtable->lastentry; p++) {
		if (strcmp(symtable->entry[p].lexptr, s) == 0) {
			return p;
		}
	}
	return -1;
}

// insert new symbol
symtable_index symtable_insert(const char s[], int tok) {

	// entry to be returned
	struct entry *symentry;

	// entry already in sym table - return it
	int index = symtable_lookup(s);
	if (index >= 0) {
		return index;
	}

	// make sure sym table has space
	if (symtable->lastentry +1 >= SYMMAX) {
		printf("symbol table full\n");
		return 0;
	}

	// add new entry to symbol table

	// entry's index
	symtable->lastentry++;

	// create new entry
	symentry = &symtable->entry[symtable->lastentry];
	symentry->token = tok;
	symentry->isRef = 0;
	symentry->scope = 0;
	strncpy(symentry->lexptr, s, ENTRY_LEXPTR_SIZE -1);
	symentry->lexptr[ENTRY_LEXPTR_SIZE-1] = '\0';
	symentry->value = 0;

	return symtable->lastentry;
}

// clean current system table and all it's parents
void symtable_clean() {
	/*
	struct symtable *csymtable;
	while (symtable != 0) {
		csymtable = symtable->parent;
		free(symtable);
		symtable = csymtable;
	}
	*/
}

// print symbol table
void symtable_print() {
	for (int j = 0; j <= symtable_current; j++) {
		printf("symtable %d:\n", j);
		for (int i = 0; i <= symtable->lastentry; i++) {
			printf("\t%s | token: %d\n", symtable->entry[i].lexptr, symtable->entry[i].token);
		}
		printf("end symtable %d:\n", j);
	}
}

// get entry from index
struct entry *symtable_entryat(symtable_index index) {
	return &symtable->entry[index];
}
