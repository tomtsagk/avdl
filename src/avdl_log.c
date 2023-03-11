#include "avdl_log.h"
#include <stdarg.h>
#include <stdio.h>
#include "avdl_settings.h"

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <windows.h>
#endif

void avdl_log(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	/*
	 * avdl is currently not designed to run on android
	 * (it can compile games for android)
	#if DD_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_INFO, "avdl", msg, args);
	 */
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	/*
	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);
	MessageBox(0, buffer, "Avdl Log:", 0);
	*/
	vprintf(msg, args);
	printf("\n");
	#else
	vprintf(msg, args);
	printf("\n");
	#endif

	va_end(args);
}

void avdl_log_error(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	/*
	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);
	MessageBox(0, buffer, "Avdl Error:", 0);
	*/
	vprintf(msg, args);
	printf("\n");
	#else
	printf(AVDL_LOG_ERRORSTRING);
	vprintf(msg, args);
	printf("\n");
	#endif

	va_end(args);
}
