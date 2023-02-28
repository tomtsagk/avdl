#include "dd_log.h"
#include <stdarg.h>

#if DD_PLATFORM_ANDROID
#include <android/log.h>
#else
#include <stdio.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

void dd_log(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	#if DD_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_INFO, "avdl", msg, args);
	#elif defined(_WIN32) || defined(WIN32)
	char buffer[1024];
	vsnprintf(buff, 1024, msg, args);
	MessageBox(0, buffer, "Variable", 0);
	#else
	/*
	FILE *f = fopen("error.log", "a");
	vfprintf(f, msg, args);
	fprintf(f, "\n");
	fclose(f);
	*/
	vprintf(msg, args);
	printf("\n");
	#endif

	va_end(args);
}
