#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#include <windows.h>
#define AVDL_FILE_OPEN(x, y) _open(x, y)
#define AVDL_FILE_OPEN_MODE(x, y, z) _open(x, y, z)
#else
#include <dirent.h>
#include <unistd.h>
#define AVDL_FILE_OPEN(x, y) open(x, y)
#define AVDL_FILE_OPEN_MODE(x, y, z) open(x, y, z)
#endif

#include "avdl_file_op.h"

int file_copy(const char *src, const char *dest, int append) {
	printf("copy file: %s -> %s\n", src, dest);
	#if defined(_WIN32) || defined(WIN32)
	int s = AVDL_FILE_OPEN(src, _O_RDONLY);
	if (!s) {
		printf("can't open %s: %s\n", src, strerror(errno));
		return -1;
	}

	int dest_flags = _O_WRONLY | _O_CREAT;
	if (append) dest_flags |= _O_APPEND;
	int d = AVDL_FILE_OPEN_MODE(dest, dest_flags, _S_IREAD | _S_IWRITE);
	if (!d) {
		printf("can't open %s: %s\n", dest, strerror(errno));
		close(s);
		return -1;
	}

	char buffer[1024];

	int i = 0;
	ssize_t nread;
	while (nread = read(s, buffer, 1024), nread > 0) {
		if (write(d, buffer, nread) == -1) {
			printf("avdl error: %s\n", strerror(errno));
		}
		i++;
	}

	close(s);
	close(d);

	return 0;
	#else
	file_copy_at(0, src, 0, dest, append);
	#endif
	return 0;
}

int file_copy_at(int src_at, const char *src, int dest_at, const char *dest, int append) {
	#if defined(_WIN32) || defined(WIN32)
	#else
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
			printf("avdl error: %s\n", strerror(errno));
		}
		i++;
	}

	close(s);
	close(d);
	#endif

	return 0;
}

int file_replace(int src_at, const char *src, int dst_at, const char *dst,
	const char *findString, const char *replaceString) {

	#if defined(_WIN32) || defined(WIN32)
	#else
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
	int d;
	if (dst_at) {
		d = openat(dst_at, dst, dest_flags, S_IRUSR | S_IWUSR);
	}
	else {
		d = open(dst, dest_flags, S_IRUSR | S_IWUSR);
	}
	if (!d) {
		printf("can't open %s: %s\n", dst, strerror(errno));
		return -1;
	}

	char buffer[1024];
	char strToFind[2];
	strToFind[0] = findString[0];
	strToFind[1] = '\0';

	int i = 0;
	ssize_t nread;
	char *found;
	int drawReplaceString;

	// check chunks of file, replace all instances of string
	while (nread = read(s, buffer, 1023), nread > 0) {
		drawReplaceString = 0;
		buffer[nread] = '\0';

		// possibly found the string it is looking for
		if (found = strstr(buffer, strToFind)) {

			/*
			 * string is cutoff - can't determine if it's the one
			 * scan until the beginning of that string, and try on
			 * next iteration
			 */
			if (strlen(found) < strlen(findString)) {
				lseek(s, -nread +(found -buffer), SEEK_CUR);
				nread = found -buffer;
			}
			else
			/*
			 * found string, print everything before it, skip
			 * the string itself, and later print the replaced string
			 */
			if (strstr(buffer, findString)) {
				lseek(s, -nread +((found -buffer) +strlen(findString)), SEEK_CUR);
				nread = found -buffer;
				drawReplaceString = 1;
			}
		}

		// write everything scanned so far
		if (write(d, buffer, nread) == -1) {
			printf("error: %s\n", strerror(errno));
		}

		// found key string, so write the desired string
		if (drawReplaceString) {
			write(d, replaceString, strlen(replaceString));
		}
		i++;

	}

	close(s);
	close(d);
	#endif

	return 0;
}

int dir_copy_recursive(int src_at, const char *src, int dst_at, const char *dst) {

	#if defined(_WIN32) || defined(WIN32)
	#else
	// open source directory
	int src_dir;
	if (src_at) {
		src_dir = openat(src_at, src, O_DIRECTORY);
	}
	else {
		src_dir = open(src, O_DIRECTORY);
	}

	if (!src_dir) {
		printf("avdl error: Unable to open '%s': %s\n", src, strerror(errno));
		return -1;
	}

	// open destination directory
	int dst_dir;
	if (dst_at) {
		dst_dir = openat(dst_at, dst, O_DIRECTORY);
	}
	else {
		dst_dir = open(dst, O_DIRECTORY);
	}

	if (!dst_dir) {
		printf("avdl error: Unable to open '%s': %s\n", dst, strerror(errno));
		close(src_dir);
		return -1;
	}

	/*
	 * start reading all files from source directory
	 */
	DIR *d = fdopendir(src_dir);
	if (!d) {
		printf("avdl error: Unable to open dir '%s': %s\n", src, strerror(errno));
		close(src_dir);
		close(dst_dir);
		return -1;
	}

	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {

		// ignore `.` and `..`
		if (strcmp(dir->d_name, ".") == 0
		||  strcmp(dir->d_name, "..") == 0) {
			continue;
		}

		// check file type
		struct stat statbuf;
		if (fstatat(src_dir, dir->d_name, &statbuf, 0) != 0) {
			printf("avdl error: Unable to stat file '%s': %s\n", dir->d_name, strerror(errno));
			close(src_dir);
			close(dst_dir);
			closedir(d);
			return -1;
		}

		// is directory - copy everything recursively
		if (S_ISDIR(statbuf.st_mode)) {
			if (dir_createat(dst_dir, dir->d_name) != 0) {
				printf("avdl error: Unable to create directory '%s'\n", dir->d_name);
				close(src_dir);
				close(dst_dir);
				closedir(d);
				return -1;
			}

			if (dir_copy_recursive(src_dir, dir->d_name, dst_dir, dir->d_name) != 0) {
				printf("avdl error: Unable to copy directory '%s'\n", dir->d_name);
				close(src_dir);
				close(dst_dir);
				closedir(d);
				return -1;
			}
		}
		else
		// is regular file - copy it
		if (S_ISREG(statbuf.st_mode)) {
			if (file_copy_at(src_dir, dir->d_name, dst_dir, dir->d_name, 0) != 0) {
				printf("avdl error: Unable to copy file '%s'\n", dir->d_name);
				close(src_dir);
				close(dst_dir);
				closedir(d);
				return -1;
			}
		}
		// not supporting other file types
		else {
			printf("avdl error: Unable to determine type of file '%s' - skip\n", dir->d_name);
			continue;
		}
	}
	close(src_dir);
	close(dst_dir);
	closedir(d);
	#endif
	return 0;
}

int dir_create(const char *filename) {
	return dir_createat(0, filename);
}

int dir_createat(int dir_at, const char *filename) {
	#if defined(_WIN32) || defined(WIN32)
	#else
	mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
	if (dir_at) {
		mkdirat(dir_at, filename, mode);
	}
	else {
		mkdir(filename, mode);
	}
	#endif
	return 0;
}

int is_dir(const char *filename) {
	#if defined(_WIN32) || defined(WIN32)
	#else
	DIR *dir = opendir(filename);
	if (dir) {
		return 1;
	}
	else
	if (errno == ENOENT) {
		return 0;
	}
	else {
		return -1;
	}
	#endif
	return 0;
}
