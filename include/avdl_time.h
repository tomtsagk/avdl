#ifndef AVDL_TIME_H
#define AVDL_TIME_H

#include <time.h>

struct avdl_time {
	struct timespec start;
	struct timespec end;
	double start_sec;
	double end_sec;
	double elapsed_sec;
};

void avdl_time_start(struct avdl_time *o);
void avdl_time_end(struct avdl_time *o);
double avdl_time_getTimeDouble(struct avdl_time *o);

#endif
