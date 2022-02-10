#include "avdl_platform.h"
#include <stdio.h>
#include <stdlib.h>

#include "avdl_settings.h"


enum AVDL_PLATFORM avdl_platform;

void avdl_platform_initialise() {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	avdl_platform = AVDL_PLATFORM_WINDOWS;
	#elif AVDL_IS_OS(AVDL_OS_LINUX)
	avdl_platform = AVDL_PLATFORM_LINUX;
	#endif
}

void avdl_platform_set(enum AVDL_PLATFORM newPlatform) {
	avdl_platform = newPlatform;
}

enum AVDL_PLATFORM avdl_platform_get() {
	return avdl_platform;
}

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <windows.h>
wchar_t dynamicProjectLocationW[1000];
#endif

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
const wchar_t *avdl_getProjectLocation() {
#else
const char *avdl_getProjectLocation() {
#endif
	#ifdef AVDL_DYNAMIC_PKG_LOCATION

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	if (!dynamicProjectLocationW) {
		return L"";
	}
	else {
		return dynamicProjectLocationW;
	}
	#else
	return "";
	#endif

	#else

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	return LPKG_LOCATION;
	#else
	return PKG_LOCATION;
	#endif

	#endif
}

void avdl_initProjectLocation() {
	#ifdef AVDL_DYNAMIC_PKG_LOCATION

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	wchar_t *pointer = dynamicProjectLocationW;
	wchar_t *secondToLastSlash = 0;
	wchar_t *lastSlash = 0;
	int slashesLeft = 2;
	GetModuleFileNameW(NULL, dynamicProjectLocationW, 999);
	dynamicProjectLocationW[999] = L'\0';

	while (pointer[0] != L'\0') {
		if (pointer[0] == L'\\') {
			secondToLastSlash = lastSlash;
			lastSlash = pointer;
		}
		pointer++;
	}
	if (secondToLastSlash) {
		secondToLastSlash++;
		secondToLastSlash[0] = L'\0';
	}
	#else
	// not supported on non-windows os for now
	#endif

	#endif

}

void avdl_cleanProjectLocation() {
}
