#ifndef AVDL_PLATFORM_H
#define AVDL_PLATFORM_H

#include "avdl_settings.h"

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <windows.h>
#endif

#ifndef PKG_NAME
#define PKG_NAME "avdl"
#endif

#ifndef PKG_VERSION
#define PKG_VERSION "0.0.0-1"
#endif

#ifndef PKG_LOCATION
#define PKG_LOCATION ""
#endif

/*
 * compiling platform
 *
 * used when compiling a game to decide
 * which platform to compile for
 *
 * by default, it will try to compile for the same
 * platform as the host device
 *
 */
enum AVDL_PLATFORM {
	AVDL_PLATFORM_LINUX,
	AVDL_PLATFORM_WINDOWS,
	AVDL_PLATFORM_ANDROID,

	AVDL_PLATFORM_UNKNOWN,
};

void avdl_platform_initialise();
void avdl_platform_set(enum AVDL_PLATFORM);
enum AVDL_PLATFORM avdl_platform_get();

// project settings
#if AVDL_IS_OS(AVDL_OS_WINDOWS)
const wchar_t *avdl_getProjectLocation();
#else
const char *avdl_getProjectLocation();
#endif
void avdl_initProjectLocation();
void avdl_cleanProjectLocation();

#endif
