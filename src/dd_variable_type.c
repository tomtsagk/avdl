#include "dd_variable_type.h"
#include <string.h>

enum dd_variable_type dd_variable_type_convert(const char *type) {

	if (strcmp(type, "int") == 0) {
		return DD_VARIABLE_TYPE_INT;
	}
	else
	if (strcmp(type, "float") == 0) {
		return DD_VARIABLE_TYPE_FLOAT;
	}
	else {
		return DD_VARIABLE_TYPE_STRUCT;
	}
}
