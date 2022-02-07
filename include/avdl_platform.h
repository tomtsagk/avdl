#ifndef AVDL_PLATFORM_H
#define AVDL_PLATFORM_H

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
const char *avdl_getProjectLocation();
void avdl_initProjectLocation();
void avdl_cleanProjectLocation();

#endif
