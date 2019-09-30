#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"

// number of entries on each symtable
#define SYMMAX 100

// symtable
struct symtable {
	struct entry entry[SYMMAX];
	int lastentry;
	struct symtable *parent;
} *symtable;

/* init
 * allocate memory
 * no symbols inside
 */
void symtable_init() {
	symtable = malloc(sizeof(struct symtable));
	symtable->lastentry = -1;
	symtable->parent = 0;
}

// find symbol, and return its index
symtable_index lookup(char s[]) {
	int p;
	for (p = 0; p <= symtable->lastentry; p++) {
		if (strcmp(symtable->entry[p].lexptr, s) == 0) {
			return p;
		}
	}
	return -1;
}

// insert new symbol
symtable_index symtable_insert(char s[], int tok) {

	// entry to be returned
	struct entry *symentry;

	// entry already in sym table - return it
	int index = lookup(s);
	if (index >= 0) {
		return index;
	}

	// make sure sym table has space
	if (symtable->lastentry +1 >= SYMMAX) {
		printf("symbol table full");
		return 0;
	}

	// add new entry to symbol table

	// entry's index
	symtable->lastentry++;

	// create new entry
	symentry = &symtable->entry[symtable->lastentry];
	symentry->token = tok;
	strncpy(symentry->lexptr, s, ENTRY_LEXPTR_SIZE -1);
	symentry->lexptr[ENTRY_LEXPTR_SIZE-1] = '\0';
	symentry->value = 0;

	return symtable->lastentry;
}

// clean current system table and all it's parents
void symtable_clean() {
	struct symtable *csymtable;
	while (symtable != 0) {
		csymtable = symtable->parent;
		free(symtable);
		symtable = csymtable;
	}
}

// print symbol table
void symtable_print() {
	printf("symtable:\n");
	for (int i = 0; i <= symtable->lastentry; i++) {
		printf("\t%s | token: %d\n", symtable->entry[i].lexptr, symtable->entry[i].token);
	}
	printf("end symtable:\n");
}

// get entry from index
struct entry *symtable_entryat(symtable_index index) {
	return &symtable->entry[index];
}
