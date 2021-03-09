#include "avdl_data.h"
#include "dd_log.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * variable that defines the save directory.
 * useful mostly to set a custom save path for
 * specific distribution apps that might not
 * support save locally.
 */
char avdl_data_saveDirectory[1000] = "saves";

int avdl_data_save_internal(void *data, int data_size, const char *filename) {

	/*
	 * if needed, make all directories recursively
	 */
	char buffer[1000];
	strcpy(buffer, avdl_data_saveDirectory);
	strcat(buffer, "/");
	strcat(buffer, filename);

	char *start = buffer;
	char *end = start+1;
	while (end < buffer +strlen(buffer)) {
		while (end[0] != '/' && end[0] != '\0') end++;
		if (end[0] == '\0') break;
		end[0] = '\0';
		mkdir(buffer, 0777);
		end[0] = '/';
		end++;
	}

	/*
	 * if file fails to open for reading, bail out
	 */
	FILE *f = fopen(buffer, "w");
	if (!f) {
		dd_log("avdl: error opening file '%s': %s",
			buffer, strerror(errno)
		);
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fwrite(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			dd_log("avdl: error while closing file '%s': %s",
				buffer, strerror(errno)
			);
			exit(-1);
		}
		return -1;
	}

	/*
	 * close file and check for errors
	 */
	if (fclose(f) != 0) {
		dd_log("avdl: error while closing file '%s': %s",
			buffer, strerror(errno)
		);
		exit(-1);
	}

	// everything worked as expected
	return 0;
}

int avdl_data_load_internal(void *data, int data_size, const char *filename) {

	char buffer[1000];
	strcpy(buffer, avdl_data_saveDirectory);
	strcat(buffer, "/");
	strcat(buffer, filename);

	/*
	 * if file fails to open for reading, bail out
	 */
	FILE *f = fopen(buffer, "r");
	if (!f) {
		dd_log("avdl: error opening file '%s': %s",
			buffer, strerror(errno)
		);
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fread(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			dd_log("avdl: error while closing file '%s': %s",
				buffer, strerror(errno)
			);
			exit(-1);
		}
		return -1;
	}

	/*
	 * close file and check for errors
	 */
	if (fclose(f) != 0) {
		dd_log("avdl: error while closing file '%s': %s",
			buffer, strerror(errno)
		);
		exit(-1);
	}

	// everything worked as expected
	return 0;
}
