#ifndef DD_STRUCT_TABLE_H
#define DD_STRUCT_TABLE_H

// get variable types
#include "dd_variable_type.h"

// struct sizes
#define DD_STRUCT_TABLE_NAME_SIZE 100
#define DD_STRUCT_TABLE_MEMBER_TOTAL 100

// struct members
struct struct_table_entry_member {
	char name[DD_STRUCT_TABLE_NAME_SIZE];
	enum dd_variable_type type;
	char nametype[DD_STRUCT_TABLE_NAME_SIZE];
	int arrayCount;
	int isRef;
};

// structs
struct struct_table_entry {
	char name[DD_STRUCT_TABLE_NAME_SIZE];
	struct struct_table_entry_member members[100];
	int member_total;
	int parent;
};

// struct table
#define DD_STRUCT_TABLE_TOTAL 100
extern struct struct_table_entry struct_table[DD_STRUCT_TABLE_TOTAL];
extern int struct_table_current;

// functions
void struct_table_init();

// push/pop a struct to the table, optionally with a parent name (or `0` for none)
// same for members
int struct_table_push(const char *structname, const char *parentname);
void struct_table_pop();
struct struct_table_entry_member *struct_table_push_member(const char *name, enum dd_variable_type type, const char *nametype, int isRef);
void struct_table_push_member_array(const char *name, enum dd_variable_type type, const char *nametype, int arrayCount, int isRef);

// print all structs and their members -- this is meant for debug only
void struct_table_print();

// get name of struct, or name of member of struct, on given indices
const char *struct_table_get_name(int structIndex);
int struct_table_get_index(const char *structname);
const char *struct_table_get_member_name(int structIndex, int memberIndex);
int struct_table_getMemberArrayCount(int structIndex, int memberIndex);
int struct_table_getMemberIsRef(int structIndex, int memberIndex);

// get struct member type
enum dd_variable_type struct_table_get_member_type(int structIndex, int memberIndex);

// check if member is a primitive type, primitives are int, float and string
int struct_table_is_member_primitive(int structIndex, int memberIndex);
int struct_table_is_member_primitive_string(int structIndex, const char *membername);

// get index of member
int struct_table_get_member(int structIndex, const char *membername);

// check if the given member is a member of one of the parents
int struct_table_has_member(int structIndex, const char *membername);
int struct_table_has_member_parent(int structIndex, const char *membername);
int struct_table_is_member_parent(int structIndex, const char *membername);

int struct_table_get_member_scope(int structIndex, int memberIndex);
int struct_table_get_member_scope_string(int structIndex, const char *memberIndex);

int struct_table_get_parent(int structIndex);
unsigned int struct_table_get_member_total(int structIndex);
char *struct_table_get_member_nametype(int structIndex, int memberIndex);

// getters
int struct_table_count();

int struct_table_exists(const char *structname);

#endif
