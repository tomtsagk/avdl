#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

// type for index on the system table
typedef int symtable_index;

/* entry in the symbol table and its data
 */
#define ENTRY_LEXPTR_SIZE 100
struct entry {
	char lexptr[ENTRY_LEXPTR_SIZE];
	int token;
	int value;
};

// init
void symtable_init();

// lookup and insert functions
symtable_index symtable_lookup(char s[]);
symtable_index symtable_insert(char s[], int tok);

/* scope functions
 * push: create a new (empty) symtable as a child of the current one
 * pop : remove current symtable (including clean) and assign parent as the current one
 */
void symtable_push();
void symtable_pop ();

// responsible for cleaning memory
void symtable_clean();

// mostly for debugging, just print the table
void symtable_print();

// from an index to the symtable, get the entry
struct entry *symtable_entryat(symtable_index index);

#endif
