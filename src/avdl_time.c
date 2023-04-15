#include "avdl_time.h"

void avdl_time_start(struct avdl_time *o) {
	clock_gettime(CLOCK_REALTIME, &o->start);
}

#define NANO_PER_SEC 1000000000.0

void avdl_time_end(struct avdl_time *o) {
	clock_gettime(CLOCK_REALTIME, &o->end);
	o->start_sec = o->start.tv_sec + o->start.tv_nsec / NANO_PER_SEC;
	o->end_sec = o->end.tv_sec + o->end.tv_nsec / NANO_PER_SEC;
	o->elapsed_sec = o->end_sec - o->start_sec;
}

double avdl_time_getTimeDouble(struct avdl_time *o) {
	return o->elapsed_sec;
}
