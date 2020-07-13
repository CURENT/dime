/*
 * log.h - Logging routines
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
 * @file log.h
 * @brief Logging routines
 * @author Nicholas West
 * @date 2020
 *
 * Specifies miscellaneous logging routines to be used by
 * @link dime_client_t @endlink and @link dime_server_t @endlink. These
 * methods should not be called unless at least one "-v" flag was
 * specified at runtime.
 */

#include <bsd/string.h>

#ifndef __DIME_log_H
#define __DIME_log_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a formated "INFO" line to stderr
 *
 * Prints a printf-formatted line containing verbose, non-issue
 * information with a timestamp to standard error.
 *
 * @param fmt Format string
 * @param ... Additional arguments
 */
void dime_info(const char *fmt, ...);

/**
 * @brief Send a formated "WARN" line to stderr
 *
 * Prints a printf-formatted line containing non-critical warning
 * information with a timestamp to standard error.
 *
 * @param fmt Format string
 * @param ... Additional arguments
 */
void dime_warn(const char *fmt, ...);

/**
 * @brief Send a formated "ERR" line to stderr
 *
 * Prints a printf-formatted line containing critical error information
 * with a timestamp to standard error.
 *
 * @param fmt Format string
 * @param ... Additional arguments
 */
void dime_err(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
