#ifndef AVDL_TIME_H
#define AVDL_TIME_H

#include <time.h>
#include "avdl_settings.h"

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <windows.h>
#endif

struct avdl_time2 {
	#if AVDL_IS_OS(AVDL_OS_LINUX)
	struct timespec start;
	struct timespec end;
	double start_sec;
	double end_sec;
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS)
	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	#endif
	double elapsed_sec;
};

void avdl_time2_start(struct avdl_time2 *o);
void avdl_time2_end(struct avdl_time2 *o);
double avdl_time2_getTimeDouble(struct avdl_time2 *o);

#endif
