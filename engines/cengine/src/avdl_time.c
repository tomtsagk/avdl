#include "avdl_time.h"

void avdl_time_start(struct avdl_time *o) {
	#if defined(_WIN32) || defined(WIN32)
	QueryPerformanceFrequency(&o->frequency);
	QueryPerformanceCounter(&o->start);
	#else
	clock_gettime(CLOCK_REALTIME, &o->start);
	#endif
}

#define NANO_PER_SEC 1000000000.0

void avdl_time_end(struct avdl_time *o) {
	#if defined(_WIN32) || defined(WIN32)
	QueryPerformanceCounter(&o->end);
	o->elapsed_sec = (double)(o->end.QuadPart -o->start.QuadPart) / o->frequency.QuadPart;
	#else
	clock_gettime(CLOCK_REALTIME, &o->end);
	o->start_sec = o->start.tv_sec + o->start.tv_nsec / NANO_PER_SEC;
	o->end_sec = o->end.tv_sec + o->end.tv_nsec / NANO_PER_SEC;
	o->elapsed_sec = o->end_sec - o->start_sec;
	#endif
}

double avdl_time_getTimeDouble(struct avdl_time *o) {
	#if defined(_WIN32) || defined(WIN32)
	return o->elapsed_sec;
	#else
	return o->elapsed_sec;
	#endif
	return 0;
}
