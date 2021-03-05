#include "avdl_data.h"
#include "dd_log.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int avdl_data_save_internal(void *data, int data_size, const char *filename) {

	FILE *f = fopen(filename, "w");

	/*
	 * if file fails to open for reading, bail out
	 */
	if (!f) {
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fwrite(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			dd_log("avdl: error while closing file '%s': %s",
				filename, strerror(errno)
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
			filename, strerror(errno)
		);
		exit(-1);
	}

	// everything worked as expected
	return 0;
}

int avdl_data_load_internal(void *data, int data_size, const char *filename) {

	FILE *f = fopen(filename, "r");

	/*
	 * if file fails to open for reading, bail out
	 */
	if (!f) {
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fread(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			dd_log("avdl: error while closing file '%s': %s",
				filename, strerror(errno)
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
			filename, strerror(errno)
		);
		exit(-1);
	}

	// everything worked as expected
	return 0;
}
