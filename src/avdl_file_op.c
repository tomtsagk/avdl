#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "avdl_file_op.h"
#include "avdl_settings.h"
#include "avdl_string.h"
#include "avdl_log.h"

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <io.h>
#include <windows.h>
#define AVDL_FILE_OPEN(x, y) _open(x, y)
#define AVDL_FILE_OPEN_MODE(x, y, z) _open(x, y, z)
#define AVDL_FILE_MKDIR(filename, mode) _mkdir(filename)
#else
#include <dirent.h>
#include <unistd.h>
#define AVDL_FILE_OPEN(x, y) open(x, y)
#define AVDL_FILE_OPEN_MODE(x, y, z) open(x, y, z)
#define AVDL_FILE_MKDIR(filename, mode) mkdir(filename, mode)
#endif

int file_copy(const char *src, const char *dest, int append) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
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
	long int nread;
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
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
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

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	//return file_copy_at(src_at, src, dest_at, dest, 0);
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
		found = strstr(buffer, strToFind);
		if (found) {

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

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
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
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	if (!dir_at) {
		AVDL_FILE_MKDIR(filename, mode);
	}
	else {
		printf("avdl error: cannot create directories on specific locations on windows yet\n");
		return -1;
	}
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
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else

	// doesn't exist - not directory
	if ( !Avdl_FileOp_DoesFileExist(filename) ) {
		return 0;
	}

	// check file type
	struct stat statbuf;
	if ( stat(filename, &statbuf) != 0 ) {
		printf("avdl error: Unable to stat file '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// is directory - copy everything recursively
	if (S_ISDIR(statbuf.st_mode)) {
		return 1;
	}
	#endif
	return 0;
}

int Avdl_FileOp_GetNumberOfFiles(const char *directory) {

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 0;
	#else
	// only handle directories
	if (!is_dir(directory)) {
		printf("avdl error: not a directory: %s\n", directory);
		return -1;
	}

	// open source directory
	int src_dir = open(directory, O_DIRECTORY);
	if (!src_dir) {
		printf("avdl error: Unable to open '%s': %s\n", directory, strerror(errno));
		return -1;
	}

	DIR *d = opendir(directory);
	if (!d) {
		printf("avdl error: Unable to open directory '%s': %s\n", directory, strerror(errno));
		return -1;
	}

	int files = 0;

	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		struct stat statbuf;
		if ( fstatat(src_dir, dir->d_name, &statbuf, 0) != 0 ) {
			printf("avdl error: Unable to stat directory '%s': %s\n", directory, strerror(errno));
			closedir(d);
			return -1;
		}

		if (S_ISREG(statbuf.st_mode)) {
			files++;
		}
	}

	closedir(d);
	return files;
	#endif
}

int Avdl_FileOp_ForFileInDirectory(const char *dirname, int (*handle_function)(const char *dirname, const char *filename, int, int)) {

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", dirname);

	if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		avdl_log_error("Path not found: [%s]\n", dirname);
		return -1;
	}

	do
	{

		//Find first file will always return "."
		//    and ".." as the first two directories.
		if(strcmp(fdFile.cFileName, ".") == 0
		|| strcmp(fdFile.cFileName, "..") == 0) {
			continue;
		}

		//Build up our file path using the passed in
		//  [sDir] and the file/foldername we just found:
		sprintf(sPath, "%s\\%s", dirname, fdFile.cFileName);

		//Is the entity a File or Folder?
		if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
		{
			//avdl_log("Directory: %s\n", sPath);
			//ListDirectoryContents(sPath); //Recursion, I love it!
		}
		else{
			//avdl_log("File: %s\n", sPath);
			handle_function(dirname, fdFile.cFileName, 5, 10);
		}
	}
	while(FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return 0;
	#else

	int files_to_handle = Avdl_FileOp_GetNumberOfFiles(dirname);
	int files_handled = 0;

	/*
	 * start reading all files from source directory
	 */
	DIR *d = opendir(dirname);
	if (!d) {
		printf("avdl error: Unable to open directory '%s': %s\n", dirname, strerror(errno));
		return -1;
	}

	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		files_handled++;

		// given function returned a non zero value, it's unclear if this is
		// a fatal error or not, so don't print anything, let the calling
		// function handle it
		if ( handle_function(dirname, dir->d_name, files_handled, files_to_handle) != 0 ) {
			closedir(d);
			return -1;
		}
	}

	closedir(d);
	return 0;
	#endif
}

int Avdl_FileOp_GetFilesInDirectory(const char *dirname, struct dd_dynamic_array *array) {

	dd_da_init(array, sizeof(struct avdl_string));

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", dirname);

	if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		avdl_log_error("Path not found: [%s]\n", dirname);
		return -1;
	}

	do
	{

		//Find first file will always return "."
		//    and ".." as the first two directories.
		if(strcmp(fdFile.cFileName, ".") == 0
		|| strcmp(fdFile.cFileName, "..") == 0) {
			continue;
		}

		//Build up our file path using the passed in
		//  [sDir] and the file/foldername we just found:
		sprintf(sPath, "%s\\%s", dirname, fdFile.cFileName);

		//Is the entity a File or Folder?
		if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
		{
			//avdl_log("Directory: %s\n", sPath);
			//ListDirectoryContents(sPath); //Recursion, I love it!
		}
		else{

			// add a string into the array and get a pointer to it
			struct avdl_string str;
			dd_da_push(array, &str);
			struct avdl_string *str2 = dd_da_get(array, -1);

			// put filename into the string
			avdl_string_create(str2, 1024);
			avdl_string_cat(str2, fdFile.cFileName);
			if ( !avdl_string_isValid(str2) ) {
				avdl_log_error("Unable to collect filename");
				return -1;
			}
		}
	}
	while(FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return 0;
	#else

	/*
	 * start reading all files from source directory
	 */
	DIR *d = opendir(dirname);
	if (!d) {
		avdl_log_error("Unable to open directory '%s': %s\n", dirname, strerror(errno));
		return -1;
	}

	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {

		// add a string into the array and get a pointer to it
		struct avdl_string str;
		dd_da_push(array, &str);
		struct avdl_string *str2 = dd_da_get(array, -1);

		// put filename into the string
		avdl_string_create(str2, 1024);
		avdl_string_cat(str2, dir->d_name);
		if ( !avdl_string_isValid(str2) ) {
			avdl_log_error("Unable to collect filename");
			return -1;
		}

	}

	closedir(d);
	return 0;
	#endif
	return 0;
}

int Avdl_FileOp_DoesFileExist(const char *filename) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 0;
	#else
	return access(filename, F_OK) == 0;
	#endif
}

int Avdl_FileOp_IsDirStat(struct stat *s) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 0;
	#else
	return S_ISDIR(s->st_mode);
	#endif
}

int Avdl_FileOp_IsRegStat(struct stat *s) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 1;
	#else
	return S_ISREG(s->st_mode);
	#endif
}

int file_remove(const char *filename) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 0;
	#else
	remove(filename);
	return 0;
	#endif
}

int Avdl_FileOp_IsFileOlderThan(const char *source, const char *target) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return 1;
	#else

	// source file doesn't exist - target is newer by default
	if ( !Avdl_FileOp_DoesFileExist(source) ) {
		return 1;
	}

	// target is a directory - go recursive
	if ( is_dir(target) ) {
		int isNewer = 0;

		/*
		 * start reading all files from source directory
		 */
		DIR *d = opendir(target);
		if (!d) {
			printf("avdl error: Unable to open directory '%s': %s\n", target, strerror(errno));
			return -1;
		}

		struct dirent *dir;
		char dirBuffer[1024];
		while ((dir = readdir(d)) != NULL) {

			// ignore `.` and `..`
			if(strcmp(dir->d_name, ".") == 0
			|| strcmp(dir->d_name, "..") == 0) {
				continue;
			}

			strcpy(dirBuffer, target);
			strcat(dirBuffer, "/");
			strcat(dirBuffer, dir->d_name);
			if ( Avdl_FileOp_IsFileOlderThan(source, dirBuffer) ) {
				isNewer = 1;
				break;
			}
		}
		closedir(d);
		return isNewer;
	}

	struct stat statbuffer;
	if (stat(source, &statbuffer) != 0) {
		printf("avdl error: Unable to stat file '%s': %s\n", source, strerror(errno));
		return -1;
	}

	struct stat statbuffer2;
	if (stat(target, &statbuffer2) != 0) {
		printf("avdl error: Unable to stat second file '%s': %s\n", target, strerror(errno));
		return -1;
	}

	if (difftime(statbuffer2.st_mtime, statbuffer.st_mtime) >= 0) {
		return 1;
	}
	return 0;

	#endif
}

int file_write(const char *filename, const char *content, int append) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	FILE *f = fopen(filename, "w");
	if (f == NULL)
	{
		printf("can't open %s: %s\n", filename, strerror(errno));
		return -1;
	}

	fprintf(f, "%s", content);

	fclose(f);

	return 0;
	#else
	int flags = O_WRONLY | O_CREAT;
	if (append) {
		flags |= O_APPEND;
	}
	int d;
	d = open(filename, flags, S_IRUSR | S_IWUSR);
	if (!d) {
		printf("can't open %s: %s\n", filename, strerror(errno));
		return -1;
	}

	if (write(d, content, strlen(content)) == -1) {
		printf("avdl error: %s\n", strerror(errno));
	}

	close(d);
	#endif
	return 0;
}
