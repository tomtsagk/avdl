#include "dd_variable_type.h"
#include <string.h>
#include "struct_table.h"
#include <stdio.h>

enum dd_variable_type dd_variable_type_convert(const char *type) {

	/*
	 * primitive types
	 */
	if (strcmp(type, "int") == 0) {
		return DD_VARIABLE_TYPE_INT;
	}
	else
	if (strcmp(type, "float") == 0) {
		return DD_VARIABLE_TYPE_FLOAT;
	}
	else
	if (strcmp(type, "char") == 0) {
		return DD_VARIABLE_TYPE_CHAR;
	}
	else
	if (strcmp(type, "void") == 0) {
		return DD_VARIABLE_TYPE_VOID;
	}

	/*
	 * structs
	for (int i = 0; i <= struct_table_current; i++) {
		if (strcmp(type, struct_table_get_name(i)) == 0) {
			return DD_VARIABLE_TYPE_STRUCT;
		}
	}
	 */

	// temporarily return struct for anything unknown
	return DD_VARIABLE_TYPE_STRUCT;

	/*
	 * unrecognized type
	return DD_VARIABLE_TYPE_UNKNOWN;
	 */

}

int dd_variable_type_isPrimitiveType(const char *type) {
	if (strcmp(type, "int"  ) == 0
	||  strcmp(type, "float") == 0
	||  strcmp(type, "char" ) == 0) {
		return 1;
	}
	return 0;
}

const char *dd_variable_type_getString(enum dd_variable_type type) {
	switch (type) {
	case DD_VARIABLE_TYPE_INT: return "int";
	case DD_VARIABLE_TYPE_FLOAT: return "float";
	case DD_VARIABLE_TYPE_CHAR: return "char";
	case DD_VARIABLE_TYPE_STRUCT: return "struct";
	case DD_VARIABLE_TYPE_STRING: return "string";
	case DD_VARIABLE_TYPE_UNKNOWN: default: return "unknown";
	}
}
