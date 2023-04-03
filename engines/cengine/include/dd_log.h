#ifndef DD_LOG_H
#define DD_LOG_H

#ifdef __cplusplus
extern "C"
#endif
/*
 * dd_log
 * custom wrapper to print a message to
 * the right output, depending on the platform.
 *
 * on a native build, it prints it to `stdout`,
 * on android it prints it to logcat, with the tag `avdl`
 */
void dd_log(const char *msg, ...);

#endif
