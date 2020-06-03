#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

#define ERRSTR_SIZ 81

static pthread_key_t key;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static void destroytls(void) {
    pthread_key_destroy(&key);
}

static void inittls(void) {
    int err;

    err = pthread_key_create(&key, free);
    assert(err >= 0);

    err = atexit(destroytls);
    assert(err == 0);
}

const char *dime_errorstring(const char *fmt, ...) {
    int err;

    err = pthread_once(&once, inittls);
    assert(err >= 0);

    char *str = pthread_getspecific(&key);

    if (str == NULL) {
        str = malloc(ERRSTR_SIZ);
        assert(str != NULL);

        err = pthread_setspecific(&key, str);
        assert(err >= 0);

        str[0] = '\0';
    }

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);

        vsnprintf(str, ERRSTR_SIZ, fmt, args);

        va_end(args);
    }

    return str;
}
