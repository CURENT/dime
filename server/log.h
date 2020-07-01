/*
 * error.h - Error string
 * Copyright (c) 2020 Nicholas West, Hantao Cui, CURENT, et. al.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This software is provided "as is" and the author disclaims all
 * warranties with regard to this software including all implied warranties
 * of merchantability and fitness. In no event shall the author be liable
 * for any special, direct, indirect, or consequential damages or any
 * damages whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action, arising
 * out of or in connection with the use or performance of this software.
 */

/**
 * @file error.h
 * @brief Error string
 * @author Nicholas West
 * @date 2020
 */

#ifndef __DIME_log_H
#define __DIME_log_H

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__))
/* Shamelessly stolen from OpenBSD */

static size_t strlcpy(char *restrict dst, const char *restrict src, size_t dsize) {
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') {
                break;
            }
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0) {
            *dst = '\0'; /* NUL-terminate dst */
        }

        while (*src++);
    }

    return src - osrc - 1; /* count does not include NUL */
}

static size_t strlcat(char *dst, const char *src, size_t dsize) {
    const char *odst = dst;
    const char *osrc = src;
    size_t n = dsize;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end. */
    while (n-- != 0 && *dst != '\0') {
        dst++;
    }
    dlen = dst - odst;
    n = dsize - dlen;

    if (n-- == 0) {
        return dlen + strlen(src);
    }
    while (*src != '\0') {
        if (n != 0) {
            *dst++ = *src;
            n--;
        }
        src++;
    }
    *dst = '\0';

    return dlen + (src - osrc); /* count does not include NUL */
}

#endif

void dime_info(const char *fmt, ...);

void dime_warn(const char *fmt, ...);

void dime_err(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
