#include "shared/avdl_log.h"
#include <stdarg.h>
#include <stdio.h>
#include "avdl_settings.h"

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

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	__android_log_vprint(ANDROID_LOG_INFO, "avdl", msg, args);
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS) || defined( AVDL_WINDOWS )
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

#endif // !defined(AVDL_DIRECT3D11)

void avdl_log_error(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	__android_log_vprint(ANDROID_LOG_ERROR, "avdl", msg, args);
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS) || defined( AVDL_WINDOWS )
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
