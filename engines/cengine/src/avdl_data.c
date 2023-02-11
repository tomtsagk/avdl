#include "avdl_data.h"
#include "dd_log.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj_core.h>
#include <Objbase.h>
#else
#include <unistd.h>
#endif

/*
 * variable that defines the save directory.
 * useful mostly to set a custom save path for
 * specific distribution apps that might not
 * support save locally.
 */
char avdl_data_saveDirectory[1024] = "saves";

void parse_filename(char *buffer, const char *filename) {
	#if defined(_WIN32) || defined(WIN32)
	//dd_log("parsing: %s", filename);

	/*
	PWSTR *path;
	SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, path);
	wprintf(L"path: %ls\n", path);
	CoTaskMemFree(path);
	*/

	return;

	#elif defined(DD_PLATFORM_ANDROID)

	strcpy(buffer, avdl_data_saveDirectory);
	strcat(buffer, "/");
	strcat(buffer, filename);

	#else
	buffer[0] = '\0';
	int bufferC = 0;
	int c = 0;
	while (filename[c] != '\0' && bufferC < 1023) {

		if (filename[c] == '~') {
			strcat(buffer, getenv("HOME"));
			bufferC = strlen(buffer);
		}
		else {
			buffer[bufferC] = filename[c];
			bufferC++;
		}
		c++;
	}
	buffer[bufferC] = '\0';
	#endif
}

int avdl_data_save_internal(void *data, int data_size, const char *filename) {

	#if defined(_WIN32) || defined(WIN32)

	PWSTR path;
	SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &path);

	int pathLen = wcslen(path) +wcslen(L"\\rue\\") +strlen(filename) +1;
	wchar_t *finalLoc = malloc(sizeof(wchar_t) *pathLen);
	wcscpy(finalLoc, path);
	wcscat(finalLoc, L"\\rue\\");
	//wcscat(finalLoc, filename);
	mbstowcs((finalLoc +wcslen(finalLoc)), filename, strlen(filename));
	finalLoc[pathLen-1] = L'\0';
	//wprintf(L"path final to save: %lS\n", finalLoc);

	// make all needed directories
	wchar_t *start = finalLoc;
	wchar_t *end = start+1;
	while (end[0] != L'\0') {
		while (end[0] != L'\\' && end[0] != L'\0') end++;
		if (end[0] == '\0') break;
		end[0] = L'\0';
		_wmkdir(start);
		end[0] = L'\\';
		end++;
	}

	/*
	 * if file fails to open for reading, bail out
	 */
	FILE *f = _wfopen(finalLoc, L"w");
	if (!f) {
		wprintf(L"avdl: error opening file '%lS': %lS\n", finalLoc, _wcserror(errno));
		free(finalLoc);
		CoTaskMemFree(path);
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fwrite(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			wprintf(L"avdl: error while closing file '%lS': %lS\n", finalLoc, _wcserror(errno));
		}
		free(finalLoc);
		CoTaskMemFree(path);
		return -1;
	}

	/*
	 * close file and check for errors
	 */
	if (fclose(f) != 0) {
		wprintf(L"avdl: error while closing file '%lS': %lS\n", finalLoc, _wcserror(errno));
		return -1;
	}

	free(finalLoc);
	CoTaskMemFree(path);
	return 0;

	#else

	/*
	 * if needed, make all directories recursively
	 */
	char buffer[1024];
	parse_filename(buffer, filename);
	//strcpy(buffer, avdl_data_saveDirectory);
	//strcat(buffer, "/");
	//strcpy(buffer, filename);
	//buffer[1023] = '\0';

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

	#endif
}

int avdl_data_load_internal(void *data, int data_size, const char *filename) {

	#if defined(_WIN32) || defined(WIN32)

	//dd_log("save file: %s", filename);

	PWSTR path;
	SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &path);

	int pathLen = wcslen(path) +wcslen(L"\\rue\\") +strlen(filename) +1;
	wchar_t *finalLoc = malloc(sizeof(wchar_t) *pathLen);
	wcscpy(finalLoc, path);
	wcscat(finalLoc, L"\\rue\\");
	//wcscat(finalLoc, filename);
	mbstowcs((finalLoc +wcslen(finalLoc)), filename, strlen(filename));
	finalLoc[pathLen-1] = L'\0';
	//wprintf(L"path final: %lS\n", finalLoc);

	/*
	 * if file fails to open, that means there are no save data
	 */
	FILE *f = _wfopen(finalLoc, L"r");
	if (!f) {
		//wprintf(L"avdl: error opening file '%lS': %lS\n", finalLoc, _wcserror(errno));
		free(finalLoc);
		CoTaskMemFree(path);
		return -1;
	}

	/*
	 * if it fails to read data, also bail out
	 */
	if (fread(data, data_size, 1, f) == 0) {
		if (fclose(f) != 0) {
			wprintf(L"avdl: error closing file '%lS': %lS\n", finalLoc, _wcserror(errno));
		}
		free(finalLoc);
		CoTaskMemFree(path);
		return -1;
	}

	/*
	 * close file and check for errors
	 */
	if (fclose(f) != 0) {
		wprintf(L"avdl: error closing file 2 '%lS': %lS\n", finalLoc, _wcserror(errno));
		free(finalLoc);
		CoTaskMemFree(path);
		return -1;
	}

	free(finalLoc);
	CoTaskMemFree(path);
	return 0;
	#else

	char buffer[1024];
	parse_filename(buffer, filename);
	//buffer[1023] = '\0';
	/*
	strcpy(buffer, avdl_data_saveDirectory);
	strcat(buffer, "/");
	strcat(buffer, filename);
	*/

	/*
	 * if file fails to open for reading, bail out
	 */
	FILE *f = fopen(buffer, "r");
	if (!f) {
		/*
		dd_log("avdl: error opening file '%s': %s",
			buffer, strerror(errno)
		);
		*/
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

	#endif
}
