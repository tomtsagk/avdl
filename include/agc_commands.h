#ifndef AGC_COMMANDS_H
#define AGC_COMMANDS_H

#include "ast_node.h"

int agc_commands_isNative(const char *cmdname);

/*
 * compiling platform
 */
enum AVDL_PLATFORM {
	AVDL_PLATFORM_NATIVE,
	AVDL_PLATFORM_ANDROID,
};
extern enum AVDL_PLATFORM avdl_platform;

#endif
