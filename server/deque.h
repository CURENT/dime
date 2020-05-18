/*
 * table.h - Hash table
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
 * @file table.h
 * @brief Hash table
 * @author Nicholas West
 * @date 2020
 */

#include <stddef.h>

#ifndef __DIME_deque_H
#define __DIME_deque_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t len;

    void **arr;
    size_t cap;

    size_t begin;
    size_t end;
} dime_deque_t;

int deque_ringbuffer_init(dime_deque_t *deck);

void deque_ringbuffer_destroy(dime_deque_t *deck);

int deque_ringbuffer_pushl(dime_deque_t *deck, void *p);

int deque_ringbuffer_pushr(dime_deque_t *deck, void *p);

void *deque_ringbuffer_popl(dime_deque_t *deck);

void *deque_ringbuffer_popr(dime_deque_t *deck);

size_t deque_ringbuffer_len(dime_deque_t *deck);

#ifdef __cplusplus
}
#endif

#endif
