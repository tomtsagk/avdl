#ifndef DD_STRUCT_TABLE_H
#define DD_STRUCT_TABLE_H

// struct types
enum struct_table_type {
	DD_STRUCT_TABLE_TYPE_INT,
	DD_STRUCT_TABLE_TYPE_FLOAT,
	DD_STRUCT_TABLE_TYPE_STRUCT,
	DD_STRUCT_TABLE_TYPE_STRING,
};

// struct sizes
#define DD_STRUCT_TABLE_NAME_SIZE 100
#define DD_STRUCT_TABLE_MEMBER_TOTAL 100

// struct members
struct struct_table_entry_member {
	char name[DD_STRUCT_TABLE_NAME_SIZE];
	enum struct_table_type type;
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
struct struct_table_entry struct_table[DD_STRUCT_TABLE_TOTAL];
int struct_table_current;

// functions
void struct_table_init();
int struct_table_push(const char *structname, const char *parentname);
void struct_table_push_member(const char *name, enum struct_table_type type);
void struct_table_pop();
void struct_table_print();

const char *struct_table_get_name(int index);
const char *struct_table_get_member_name(int structIndex, int memberIndex);
int struct_table_get_member_type(int structIndex, int memberIndex);
int struct_table_is_member_primitive(int structIndex, int memberIndex);

int struct_table_get_member(int structIndex, const char *membername);

#endif
