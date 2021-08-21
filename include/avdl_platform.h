#ifndef AVDL_PLATFORM_H
#define AVDL_PLATFORM_H

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

#endif
