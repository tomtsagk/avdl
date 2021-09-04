#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#include <stdio.h>

int file_copy(const char *src, const char *dest, int append);
int file_copy_at(int src_at, const char *src, int dest_at, const char *dest, int append);
int file_replace(int src_at, const char *src, int dst_at, const char *dst,
	const char *findString, const char *replaceString);

int dir_copy_recursive(int src_at, const char *src, int dst_at, const char *dst);
int dir_create(const char *filename);
int dir_createat(int dir_at, const char *filename);
int is_dir(const char *filename);

#endif
