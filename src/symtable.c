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

	// sym table about to get a new symbol - prepare it
	int len;
	len = strlen(s);

	// entry's index
	symtable->lastentry++;

	// create new entry
	symentry = &symtable->entry[symtable->lastentry];
	symentry->token = tok;
	symentry->lexptr = malloc(sizeof(char) *len +1);
	symentry->value = 0;
	strcpy(symentry->lexptr, s);

	return symtable->lastentry;
}

// clean current system table and all it's parents
void symtable_clean() {
	struct symtable *csymtable;
	while (symtable != 0) {
		csymtable = symtable->parent;
		for (int i = 0; i <= symtable->lastentry; i++) {
			free(symtable->entry[i].lexptr);
		}
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
