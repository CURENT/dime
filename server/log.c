#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static void dime_print(const char *typestr, const char *fmt, va_list args) {
    time_t t;
    struct tm timeval;

    time(&t);
    localtime_r(&t, &timeval);

    char timestr[32], outstr[161];

    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %T %z", &timeval);
    vsnprintf(outstr, sizeof(outstr), fmt, args);

    fprintf(stderr, "[%s %s] %s\n", typestr, timestr, outstr);
}

void dime_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dime_print("\033[34mINFO\033[0m", fmt, args);

    va_end(args);
}

void dime_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dime_print("\033[33mWARN\033[0m", fmt, args);

    va_end(args);
}

void dime_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dime_print("\033[31mERR \033[0m", fmt, args);

    va_end(args);
}
