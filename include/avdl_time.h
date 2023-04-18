#ifndef AVDL_TIME_H
#define AVDL_TIME_H

#include <time.h>

struct avdl_time {
	#ifdef AVDL_IS_OS(AVDL_OS_LINUX)
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

void avdl_time_start(struct avdl_time *o);
void avdl_time_end(struct avdl_time *o);
double avdl_time_getTimeDouble(struct avdl_time *o);

#endif
