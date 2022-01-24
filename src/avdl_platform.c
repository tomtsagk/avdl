#include "avdl_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <whereami.h>

enum AVDL_PLATFORM avdl_platform;

void avdl_platform_initialise() {
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	avdl_platform = AVDL_PLATFORM_WINDOWS;
	#elif __linux__
	avdl_platform = AVDL_PLATFORM_LINUX;
	#endif
}

void avdl_platform_set(enum AVDL_PLATFORM newPlatform) {
	avdl_platform = newPlatform;
}

enum AVDL_PLATFORM avdl_platform_get() {
	return avdl_platform;
}

char *dynamicProjectLocation = 0;

const char *avdl_getProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	if (!dynamicProjectLocation) {
		return "";
	}
	else {
		return dynamicProjectLocation;
	}
	#else
	return PKG_LOCATION;
	#endif
}

void avdl_initProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	int length = wai_getExecutablePath(0, 0, 0);
	dynamicProjectLocation = malloc(sizeof(char) *(length+1));
	if (!dynamicProjectLocation) {
		printf("avdl error: unable to allocate memory for dynamic package location\n");
	}
	wai_getExecutablePath(dynamicProjectLocation, length, 0);
	dynamicProjectLocation[length] = '\0';

	int directoriesToSkip = 1;
	for (int i = length-1; i >= 0; i--) {
		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		if (dynamicProjectLocation[i] == '\\') {
		#else
		if (dynamicProjectLocation[i] == '/') {
		#endif
			if (directoriesToSkip > 0) {
				directoriesToSkip--;
			}
			else {
				dynamicProjectLocation[i+1] = '\0';
				directoriesToSkip = -1;
				break;
			}
		}
	}

	if (directoriesToSkip != -1) {
		printf("avdl error: unable to parse dynamic package location: %s\n", dynamicProjectLocation);
		free(dynamicProjectLocation);
		dynamicProjectLocation = 0;
		return;
	}
	#endif
}

void avdl_cleanProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION
	if (dynamicProjectLocation) {
		free(dynamicProjectLocation);
	}
	#endif
}
