#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#include <stdio.h>

int file_copy(const char *src, const char *dest, int append);
int file_copy_at(int src_at, const char *src, int dest_at, const char *dest, int append);
int file_replace(int src_at, const char *src, int dst_at, const char *dst,
	const char *findString, const char *replaceString);
int file_remove(const char *filename);

int dir_copy_recursive(int src_at, const char *src, int dst_at, const char *dst);
int dir_create(const char *filename);
int dir_createat(int dir_at, const char *filename);
int is_dir(const char *filename);

int Avdl_FileOp_GetNumberOfFiles(const char *directory);

// for each file in given directory, call function
int Avdl_FileOp_ForFileInDirectory(const char *dirname, int (*handle_function)(const char *, const char*, int, int));

int Avdl_FileOp_IsFileOlderThan(const char *filename, const char *filename2);

int Avdl_FileOp_DoesFileExist(const char *filename);

int Avdl_FileOp_IsDirStat(struct stat s);
int Avdl_FileOp_IsRegStat(struct stat s);

#endif
