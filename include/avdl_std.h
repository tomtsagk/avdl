#ifndef AVDL_STD_H
#define AVDL_STD_H

#include <stddef.h>

extern void *(*avdl_malloc)(size_t);
extern void *(*avdl_realloc)(void *, size_t);
extern void  (*avdl_free)(void *);

#endif
