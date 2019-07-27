#include "file_op.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

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
		return -1;
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
		return -1;
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

int dir_copy_recursive(int src_at, const char *src, int dst_at, const char *dst) {
	int src_dir;
	if (src_at) {
		src_dir = openat(src_at, src, O_DIRECTORY);
	}
	else {
		src_dir = open(src, O_DIRECTORY);
	}
	if (!src_dir) {
		printf("unable to open: %s\n", src);
		return -1;
	}

	DIR *d = fdopendir(src_dir);
	if (!d) {
		printf("unable to open dir: %s\n", src);
		return -1;
	}

	int dst_dir;
	if (dst_at) {
		dst_dir = openat(dst_at, dst, O_DIRECTORY);
	}
	else {
		dst_dir = open(dst, O_DIRECTORY);
	}

	if (!dst_at) {
		printf("unable to open: %s\n", dst);
	}

	struct dirent *dir;

	while ((dir = readdir(d)) != NULL) {
		if (strcmp(dir->d_name, ".") == 0
		||  strcmp(dir->d_name, "..") == 0) {
			continue;
		}
		file_copy_at(src_dir, dir->d_name, dst_dir, dir->d_name, 0);
	}
	close(src_dir);
	close(dst_dir);
	closedir(d);
	return 0;
}

int dir_create(const char *filename) {
	mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}
