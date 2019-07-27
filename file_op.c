#include "file_op.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int file_copy(const char *src, const char *dest, int append) {
	file_copy_at(0, src, 0, dest, append);
}

int file_copy_at(int src_at, const char *src, int dest_at, const char *dest, int append) {
	int s;
	if (src_at) {
		s = openat(src_at, src, O_RDONLY);
	}
	else {
		s = open(src, O_RDONLY);
	}
	if (!s) {
		printf("can't open %s: %s\n", src, strerror(errno));
	}

	int dest_flags = O_WRONLY | O_CREAT;
	if (append) dest_flags |= O_APPEND;
	int d;
	if (dest_at) {
		d = openat(dest_at, dest, dest_flags, S_IRUSR | S_IWUSR);
	}
	else {
		d = open(dest, dest_flags, S_IRUSR | S_IWUSR);
	}
	if (!d) {
		printf("can't open %s: %s\n", dest, strerror(errno));
	}

	char buffer[1024];

	int i = 0;
	ssize_t nread;
	while (nread = read(s, buffer, 1024), nread > 0) {
		if (write(d, buffer, nread) == -1) {
			printf("error: %s\n", strerror(errno));
		}
		i++;
	}

	close(s);
	close(d);

	return 0;
}

int dir_create(const char *filename) {
	mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}
