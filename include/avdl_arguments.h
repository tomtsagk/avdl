#ifndef AVDL_ARGUMENTS_H
#define AVDL_ARGUMENTS_H

#include "avdl_settings.h"

/*
 * Handles arguments given to this program.
 * Returns 0 if everything went ok
 * Returns a positive value if everything went ok but program needs to exit
 * Returns a negative value in case of error
 */
int avdl_arguments_handle(struct AvdlSettings *avdl_settings, int argc, char *argv[]);

#endif
