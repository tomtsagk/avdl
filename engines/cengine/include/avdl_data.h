#ifndef AVDL_DATA_H
#define AVDL_DATA_H

#if defined(_WIN32) || defined(WIN32)
#include <shlobj_core.h>
#include <Objbase.h>
#endif

extern char avdl_data_saveDirectory[];

#if defined(_WIN32) || defined(WIN32)
int avdl_data_save_internal(void *data, int data_size, const wchar_t *filename);
int avdl_data_load_internal(void *data, int data_size, const wchar_t *filename);
#else
int avdl_data_save_internal(void *data, int data_size, const char *filename);
int avdl_data_load_internal(void *data, int data_size, const char *filename);
#endif

#define avdl_data_save(data, class, filename) avdl_data_save_internal(data, sizeof(struct class), filename)
#define avdl_data_load(data, class, filename) avdl_data_load_internal(data, sizeof(struct class), filename)

#endif
