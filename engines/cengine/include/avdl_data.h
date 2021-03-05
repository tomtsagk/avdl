#ifndef AVDL_DATA_H
#define AVDL_DATA_H

int avdl_data_save_internal(void *data, int data_size, const char *filename);
int avdl_data_load_internal(void *data, int data_size, const char *filename);

#define avdl_data_save(data, class, filename) avdl_data_save_internal(data, sizeof(struct class), filename)
#define avdl_data_load(data, class, filename) avdl_data_load_internal(data, sizeof(struct class), filename)

#endif
