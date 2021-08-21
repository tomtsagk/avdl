#ifndef DD_VARIABLE_TYPES_H
#define DD_VARIABLE_TYPES_H

// struct types
enum dd_variable_type {
	DD_VARIABLE_TYPE_VOID,
	DD_VARIABLE_TYPE_INT,
	DD_VARIABLE_TYPE_FLOAT,
	DD_VARIABLE_TYPE_STRUCT,
	DD_VARIABLE_TYPE_STRING,
	DD_VARIABLE_TYPE_CHAR,
	DD_VARIABLE_TYPE_FUNCTION,
	DD_VARIABLE_TYPE_UNKNOWN,
};

enum dd_variable_type dd_variable_type_convert(const char *type);
int dd_variable_type_isPrimitiveType(const char *type);
const char *dd_variable_type_getString(enum dd_variable_type type);

#endif
