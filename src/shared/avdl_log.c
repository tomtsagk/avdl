#include "shared/avdl_log.h"
#include <stdarg.h>
#include <stdio.h>
#include "avdl_settings.h"

enum log_type {
	LOG_TYPE_INFO,
	LOG_TYPE_WARNING,
	LOG_TYPE_ERROR,
	LOG_TYPE_VERBOSE,
};

static void log_internal(const char *msg, enum log_type type, va_list args) {

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	int android_code;
	if (type == LOG_TYPE_INFO) {
		android_code = ANDROID_LOG_INFO;
	}
	else
	if (type == LOG_TYPE_WARNING) {
		android_code = ANDROID_LOG_WARN;
	}
	else
	if (type == LOG_TYPE_ERROR) {
		android_code = ANDROID_LOG_ERROR;
	}
	else
	if (type == LOG_TYPE_VERBOSE) {
		android_code = ANDROID_LOG_INFO;
	}
	else {
		android_code = ANDROID_LOG_DEFAULT;
	}
	__android_log_vprint(android_code, "avdl", msg, args);
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS) || defined( AVDL_WINDOWS )
	/*
	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);
	MessageBox(0, buffer, "Avdl Log:", 0);
	*/
	vprintf(msg, args);
	printf("\n");
	#else
	if (type == LOG_TYPE_ERROR) printf(AVDL_LOG_ERRORSTRING);
	vprintf(msg, args);
	printf("\n");
	#endif

};

#if !defined(AVDL_DIRECT3D11)

#if AVDL_IS_OS(AVDL_OS_WINDOWS) || defined( AVDL_WINDOWS )
#include <windows.h>
#endif

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <android/log.h>
#else
#include <stdio.h>
#endif

void avdl_log(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	log_internal(msg, LOG_TYPE_INFO, args);
	va_end(args);
}

#endif // !defined(AVDL_DIRECT3D11)

void avdl_log_warning(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	log_internal(msg, LOG_TYPE_WARNING, args);
	va_end(args);
}

void avdl_log_error(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	log_internal(msg, LOG_TYPE_ERROR, args);
	va_end(args);
}

// Disabled on purpose for now
void avdl_log_verbose(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	//log_internal(msg, LOG_TYPE_INFO, args);
	va_end(args);
}
