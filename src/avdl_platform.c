#include "avdl_platform.h"
#include <stdio.h>

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
