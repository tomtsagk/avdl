#include "struct_table.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void struct_table_init() {
	struct_table_current = -1;
}

int struct_table_push(const char *structname, const char *parentname) {
	if (struct_table_current+1 > DD_STRUCT_TABLE_TOTAL) {
		printf("struct_table_add: max number of struct tables reached\n");
		exit(-1);
	}

	struct_table_current++;
	struct struct_table_entry *newTable = &struct_table[struct_table_current];
	strncpy(newTable->name, structname, DD_STRUCT_TABLE_NAME_SIZE -1);
	newTable->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	newTable->member_total = 0;
	newTable->parent = -1;

	if (parentname) {
		for (int i = 0; i < struct_table_current; i++) {
			if (strcmp(struct_table[i].name, parentname) == 0) {
				newTable->parent = i;
				break;
			}
		}
		if (newTable->parent < 0) {
			printf("error: struct_table_push: no parent found with name '%s'\n", parentname);
			exit(-1);
		}
	}

	return struct_table_current;
}

void struct_table_push_member(const char *name, enum struct_table_type type) {
	struct struct_table_entry *currentTable = &struct_table[struct_table_current];
	if (currentTable->member_total+1 >= DD_STRUCT_TABLE_MEMBER_TOTAL) {
		printf("struct_table_push_member: struct '%s': maximum number of members reached\n", currentTable->name);
		exit(-1);
	}
	struct struct_table_entry_member *newMember = &currentTable->members[currentTable->member_total];
	newMember->type = type;
	strncpy(newMember->name, name, DD_STRUCT_TABLE_NAME_SIZE -1);
	currentTable->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	currentTable->member_total++;
}

void struct_table_pop() {
	struct struct_table_entry *currentTable = &struct_table[struct_table_current];
	if (currentTable->member_total > 0) {
		currentTable->member_total--;
	}
}

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

const char *struct_table_get_name(int index) {
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
	return m->type != DD_STRUCT_TABLE_TYPE_STRUCT;
}
