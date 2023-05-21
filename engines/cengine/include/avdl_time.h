#ifndef AVDL_TIME_H
#define AVDL_TIME_H

#include <time.h>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

struct avdl_time {
	#if defined(_WIN32) || defined(WIN32)
	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	#else
	struct timespec start;
	struct timespec end;
	double start_sec;
	double end_sec;
	#endif
	double elapsed_sec;
};

void avdl_time_start(struct avdl_time *o);
void avdl_time_end(struct avdl_time *o);
double avdl_time_getTimeDouble(struct avdl_time *o);

#endif
