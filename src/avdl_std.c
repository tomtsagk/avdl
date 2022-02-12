#include "avdl_std.h"
#include <stdlib.h>

void *(*avdl_malloc)(size_t) = malloc;
void *(*avdl_realloc)(void *, size_t) = realloc;
void  (*avdl_free)(void *) = free;
