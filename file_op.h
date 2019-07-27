#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#include <stdio.h>

int file_copy(const char *src, const char *dest, int append);
int file_copy_at(int src_at, const char *src, int dest_at, const char *dest, int append);

int dir_create(const char *filename);

#endif
