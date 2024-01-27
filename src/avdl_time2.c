#include "avdl_time2.h"

void avdl_time2_start(struct avdl_time2 *o) {
	#if AVDL_IS_OS(AVDL_OS_LINUX)
	clock_gettime(CLOCK_REALTIME, &o->start);
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS)
	QueryPerformanceFrequency(&o->frequency);
	QueryPerformanceCounter(&o->start);
	#endif
}

#define NANO_PER_SEC 1000000000.0

void avdl_time2_end(struct avdl_time2 *o) {
	#if AVDL_IS_OS(AVDL_OS_LINUX)
	clock_gettime(CLOCK_REALTIME, &o->end);
	o->start_sec = o->start.tv_sec + o->start.tv_nsec / NANO_PER_SEC;
	o->end_sec = o->end.tv_sec + o->end.tv_nsec / NANO_PER_SEC;
	o->elapsed_sec = o->end_sec - o->start_sec;
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS)
	QueryPerformanceCounter(&o->end);
	o->elapsed_sec = (double)(o->end.QuadPart -o->start.QuadPart) / o->frequency.QuadPart;
	#endif
}

double avdl_time2_getTimeDouble(struct avdl_time2 *o) {
	#if AVDL_IS_OS(AVDL_OS_LINUX)
	return o->elapsed_sec;
	#elif AVDL_IS_OS(AVDL_OS_WINDOWS)
	return o->elapsed_sec;
	#else
	return 0;
	#endif
}
