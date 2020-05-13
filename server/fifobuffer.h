/*
 * fifobuffer.h - FIFO buffer
 * Copyright (c) 2020 Nicholas West, CURENT, et. al.
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

#include <stddef.h>
#include <sys/types.h>

#ifndef __DIME_fifobuffer_H
#define __DIME_fifobuffer_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dime_fifobuffer dime_fifobuffer_t;

dime_fifobuffer_t *dime_fifobuffer_new();

void dime_fifobuffer_free(dime_fifobuffer_t *fifo);

size_t dime_fifobuffer_read(dime_fifobuffer_t *fifo, void *p, size_t siz);

ssize_t dime_fifobuffer_write(dime_fifobuffer_t *fifo, const void *p, size_t siz);

size_t dime_fifobuffer_peek(const dime_fifobuffer_t *fifo, void *p, size_t siz);

size_t dime_fifobuffer_discard(dime_fifobuffer_t *fifo, size_t siz);

size_t dime_fifobuffer_size(const dime_fifobuffer_t *fifo);

#ifdef __cplusplus
}
#endif

#endif
