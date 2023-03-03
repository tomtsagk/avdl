#ifndef AVDL_LOG_H
#define AVDL_LOG_H

// terminal colours
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define AVDL_LOG_ERRORSTRING "avdl " RED "error" RESET ": "

void avdl_log(const char *msg, ...);
void avdl_log_error(const char *msg, ...);

#endif
