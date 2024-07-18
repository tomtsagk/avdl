#include "dd_log.h"
#include <stdarg.h>

#ifndef AVDL_DIRECT3D11

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <android/log.h>
#else
#include <stdio.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

void avdl_log(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	__android_log_vprint(ANDROID_LOG_INFO, "avdl", msg, args);
	/*
	#elif defined(_WIN32) || defined(WIN32)
	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);
	MessageBox(0, buffer, "Avdl Log:", 0);
	*/
	#else
	vprintf(msg, args);
	printf("\n");
	#endif

	va_end(args);
}

void avdl_logError(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	__android_log_vprint(ANDROID_LOG_ERROR, "avdl", msg, args);
	#elif defined(_WIN32) || defined(WIN32)
	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);
	MessageBox(0, buffer, "Avdl Log:", 0);
	#else
	vfprintf(stderr, msg, args);
	printf("\n");
	#endif

	va_end(args);
}

#endif
