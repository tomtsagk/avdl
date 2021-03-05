#include "dd_variable_type.h"
#include <string.h>
#include "struct_table.h"

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

	/*
	 * structs
	 */
	for (int i = 0; i < struct_table_count; i++) {
		if (strcmp(type, struct_table_get_name(i)) == 0) {
			return DD_VARIABLE_TYPE_STRUCT;
		}
	}

	/*
	 * unrecognized type
	 */
	return DD_VARIABLE_TYPE_UNKNOWN;

}
