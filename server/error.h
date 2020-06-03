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

#ifndef __DIME_error_H
#define __DIME_error_H

#ifdef __cplusplus
extern "C" {
#endif

const char *dime_errorstring(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
