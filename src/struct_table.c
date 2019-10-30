#include "struct_table.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// initialise the struct table index to no tables
void struct_table_init() {
	struct_table_current = -1;
}

// push a new struct to the table
int struct_table_push(const char *structname, const char *parentname) {

	// check for max number of structs
	if (struct_table_current+1 > DD_STRUCT_TABLE_TOTAL) {
		printf("struct_table_add: max number of struct tables reached\n");
		exit(-1);
	}

	// increment counter
	struct_table_current++;

	// set the values of the new struct
	struct struct_table_entry *newStruct = &struct_table[struct_table_current];
	strncpy(newStruct->name, structname, DD_STRUCT_TABLE_NAME_SIZE -1);
	newStruct->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	newStruct->member_total = 0;
	newStruct->parent = -1;

	// if a parent name was given, check if it exists, and assign as parent of the new struct
	if (parentname) {
		for (int i = 0; i < struct_table_current; i++) {
			if (strcmp(struct_table[i].name, parentname) == 0) {
				newStruct->parent = i;
				break;
			}
		}

		// no parent was found, this is an error!
		if (newStruct->parent < 0) {
			printf("error: struct_table_push: no parent found with name '%s'\n", parentname);
			exit(-1);
		}
	}

	// return the index of the new struct
	return struct_table_current;
}

/* push a new member to the last pushed struct
 * get a reference to the last pushed struct
 * check if there is space in the struct
 * add a new member to it
 */
void struct_table_push_member(const char *name, enum dd_variable_type type) {
	struct struct_table_entry *currentStruct = &struct_table[struct_table_current];
	if (currentStruct->member_total+1 >= DD_STRUCT_TABLE_MEMBER_TOTAL) {
		printf("struct_table_push_member: struct '%s': maximum number of members reached\n", currentStruct->name);
		exit(-1);
	}
	struct struct_table_entry_member *newMember = &currentStruct->members[currentStruct->member_total];
	newMember->type = type;
	strncpy(newMember->name, name, DD_STRUCT_TABLE_NAME_SIZE -1);
	currentStruct->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	currentStruct->member_total++;
}

void struct_table_pop() {
	struct struct_table_entry *currentTable = &struct_table[struct_table_current];
	if (currentTable->member_total > 0) {
		currentTable->member_total--;
	}
}

/* print all structs and their members
 * this is meant for debug only
 */
void struct_table_print() {
	for (int i = 0; i <= struct_table_current; i++) {
		struct struct_table_entry *s = &struct_table[i];
		printf("struct: %s\n", s->name);
		for (int j = 0; j < s->member_total; j++) {
			struct struct_table_entry_member *m = &s->members[j];
			printf("	member: %s\n", m->name);
		}
	}
}

// return the name of the struct on index, make sure index is in bounds
const char *struct_table_get_name(int index) {
	if (index < 0 || index > struct_table_current) {
		printf("error: struct_table_get_name: index out of bounds: %d\n", index);
		exit(-1);
	}
	return struct_table[index].name;
}

/*
const char *struct_table_get_member_name(int structIndex, int memberIndex) {
}

int struct_table_get_member_type(int structIndex, int memberIndex) {
}
*/

int struct_table_get_member(int structIndex, const char *membername) {
	for (int i = 0; i < struct_table[structIndex].member_total; i++) {
		struct struct_table_entry_member *m = &struct_table[structIndex].members[i];
		if (strcmp(m->name, membername) == 0) {
			return i;
		}
	}
	return -1;
}

int struct_table_is_member_primitive(int structIndex, int memberIndex) {
	struct struct_table_entry_member *m = &struct_table[structIndex].members[memberIndex];
	return m->type != DD_VARIABLE_TYPE_STRUCT;
}

int struct_table_has_member(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_has_member: index out of bounds: %d\n", structIndex);
		exit(-1);
	}

	struct struct_table_entry *e = &struct_table[structIndex];
	for (int i = 0; i < e->member_total; i++) {
		struct struct_table_entry_member *m = &e->members[i];
		if (strcmp(m->name, membername) == 0) {
			return 1;
		}
	}

	return 0;
}

static int parent_level, parent_level_oldest;
static int struct_table_is_member_parent_search(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_is_member_parent: index out of bounds: %d\n", structIndex);
		exit(-1);
	}
	struct struct_table_entry *t = &struct_table[structIndex];
	if (t->parent != -1) {
		parent_level++;
		if (struct_table_has_member(t->parent, membername)) {
			parent_level_oldest = parent_level;
			//return parent_level;
		}
		return struct_table_is_member_parent_search(t->parent, membername);
	}
	return parent_level_oldest;
}

int struct_table_is_member_parent(int structIndex, const char *membername) {
	parent_level = 0;
	parent_level_oldest = 0;
	return struct_table_is_member_parent_search(structIndex, membername);
}

int struct_table_get_index(const char *structname) {
	for (int i = 0; i <= struct_table_current; i++) {
		if (strcmp(struct_table[i].name, structname) == 0) {
			return i;
		}
	}
	return -1;
}

int struct_table_get_member_scope(int structIndex, int memberIndex) {
	struct struct_table_entry_member *m = &struct_table[structIndex].members[memberIndex];
	if (m->type == DD_VARIABLE_TYPE_STRUCT) {
		return struct_table_get_index(m->name);
	}
	printf("struct_table_get_member_scope: error: type is not struct\n");
	exit(-1);
}
