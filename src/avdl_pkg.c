#include "avdl_pkg.h"
#include <stdio.h>
#include <string.h>
#include "whereami.h"
#include "avdl_settings.h"

/*
 * By default build is in a dynamic location,
 * unless a package location was given
 */
#ifdef AVDL_PKG_LOCATION
int avdl_pkg_location_type = AVDL_PKG_LOCATION_TYPE_FIXED;
char *avdl_pkg_location_path = AVDL_PKG_LOCATION;
#else
int avdl_pkg_location_type = AVDL_PKG_LOCATION_TYPE_DYNAMIC;
char *avdl_pkg_location_path = 0;
#endif

int avdl_pkg_dependencies_type = AVDL_PKG_DEPENDENCIES_TYPE_DYNAMIC;

static char buffer[1024];
static char buffer2[1024];

const char *avdl_pkg_GetCenginePath() {

	if (avdl_pkg_location_type == AVDL_PKG_LOCATION_TYPE_FIXED) {
		strcpy(buffer, avdl_pkg_location_path);
	}
	else
	if (avdl_pkg_location_type == AVDL_PKG_LOCATION_TYPE_DYNAMIC) {

		// get path of binary
		int length = wai_getExecutablePath(NULL, 0, NULL);
		if (length < 400) {
			wai_getExecutablePath(buffer, length, 0);
		}
		else {
			printf("too long path of cengine\n");
			return 0;
		}

		char slash;
		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		slash = '\\';
		#else
		slash = '/';
		#endif

		// lose last two files (so `/directory/bin/avdl` becomes `/directory/`)
		char *p = buffer +length -1;
		char *lastSlash = 0;
		char *secondToLastSlash = 0;
		while (p >= buffer) {
			if (p[0] == slash) {
				if (!lastSlash) {
					lastSlash = p;
				}
				else
				if (!secondToLastSlash) {
					secondToLastSlash = p;
					break;
				}
				else {
					printf("error getting path of cengine\n");
					return 0;
				}
			}
			p--;
		}
		if (!secondToLastSlash) {
			printf("avdl error: can't truncate path of cengine\n");
			return 0;
		}
		(secondToLastSlash+1)[0] = '\0';
	}
	else {
		printf("avdl error: unknown pkg location type: %d\n", avdl_pkg_location_type);
		return 0;
	}
	strcat(buffer, "/share/avdl/cengine/");
	return buffer;
}

const char *avdl_pkg_GetProjectPath() {

	if (avdl_pkg_location_type == AVDL_PKG_LOCATION_TYPE_FIXED) {
		strcpy(buffer2, avdl_pkg_location_path);
	}
	else
	if (avdl_pkg_location_type == AVDL_PKG_LOCATION_TYPE_DYNAMIC) {

		// get path of binary
		int length = wai_getExecutablePath(NULL, 0, NULL);
		if (length < 400) {
			wai_getExecutablePath(buffer2, length, 0);
		}
		else {
			printf("too long path of cengine\n");
			return 0;
		}

		char slash;
		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		slash = '\\';
		#else
		slash = '/';
		#endif

		// lose last two files (so `/directory/bin/avdl` becomes `/directory/`)
		char *p = buffer2 +length -1;
		char *lastSlash = 0;
		char *secondToLastSlash = 0;
		while (p >= buffer2) {
			if (p[0] == slash) {
				if (!lastSlash) {
					lastSlash = p;
				}
				else
				if (!secondToLastSlash) {
					secondToLastSlash = p;
					break;
				}
				else {
					printf("error getting path of cengine\n");
					return 0;
				}
			}
			p--;
		}
		if (!secondToLastSlash) {
			printf("avdl error: can't truncate path of cengine\n");
			return 0;
		}
		(secondToLastSlash+1)[0] = '\0';
	}
	else {
		printf("avdl error: unknown pkg location type: %d\n", avdl_pkg_location_type);
		return 0;
	}
	return buffer2;
}

int avdl_pkg_IsDynamicLocation() {
	return avdl_pkg_location_type = AVDL_PKG_LOCATION_TYPE_DYNAMIC;
}

int avdl_pkg_IsFixedLocation() {
	return avdl_pkg_location_type = AVDL_PKG_LOCATION_TYPE_FIXED;
}

int avdl_pkg_IsDynamicDependencies() {
	return avdl_pkg_dependencies_type = AVDL_PKG_DEPENDENCIES_TYPE_DYNAMIC;
}

int avdl_pkg_IsFixedDependencies() {
	return avdl_pkg_dependencies_type = AVDL_PKG_DEPENDENCIES_TYPE_FIXED;
}
