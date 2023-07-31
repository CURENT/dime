/*
 * ringbuffer.h - Ring buffer
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
 * @file ringbuffer.h
 * @brief Ring buffer
 * @author Nicholas West
 * @date 2020
 *
 * Implements a first-in-first-out, (FIFO) in-memory ring buffer bor
 * byte-oriented data. Can be thought of as a pipe with an unlimited
 * internal buffer. These ring buffers are used by the sockets to store
 * partially sent/received messages.
 */

#include <stddef.h>
#include <sys/types.h>

#ifndef __DIME_ringbuffer_H
#define __DIME_ringbuffer_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ring buffer
 *
 * Should be treated as opaque; use relevant methods to access bytes in
 * the ring buffer.
 *
 * @see dime_ringbuffer_init
 * @see dime_ringbuffer_destroy
 * @see dime_ringbuffer_read
 * @see dime_ringbuffer_write
 * @see dime_ringbuffer_peek
 * @see dime_ringbuffer_discard
 */
typedef struct {
    size_t len; /* Number of bytes in the buffer */

    unsigned char *arr; /* Byte array */
    size_t cap;         /* Capacity of byte array */

    size_t begin; /* Start of readable bytes in array */
    size_t end;   /* Start of writeable bytes in array */
} dime_ringbuffer_t;

/**
 * @brief Initialize a new ring buffer
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_ringbuffer_destroy
 */
int dime_ringbuffer_init(dime_ringbuffer_t *ring);

/**
 * @brief Free resources used by a ring buffer
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 *
 * @see dime_ringbuffer_init
 */
void dime_ringbuffer_destroy(dime_ringbuffer_t *ring);

/**
 * @brief Read bytes from the ring buffer
 *
 * Equivalent to calling @link dime_ringbuffer_peek @endlink, and then
 * calling @link dime_ringbuffer_discard @endlink with the amount of bytes
 * received.
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 * @param buf Buffer to write read bytes to
 * @param siz Buffer size
 *
 * @return Number of bytes read (may be less than @em siz)
 *
 * @see dime_ringbuffer_write
 * @see dime_ringbuffer_peek
 */
size_t dime_ringbuffer_read(dime_ringbuffer_t *ring, void *buf, size_t siz);

/**
 * @brief Write bytes to the ring buffer
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 * @param buf Buffer to read written bytes from
 * @param siz Buffer size
 *
 * @return Number of bytes written, or a negative value on failure
 *
 * @see dime_ringbuffer_read
 */
ssize_t dime_ringbuffer_write(dime_ringbuffer_t *ring,
                              const void *buf,
                              size_t siz);

/**
 * @brief Read bytes from the ring buffer without advancing
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 * @param buf Buffer to write read bytes to
 * @param siz Buffer size
 *
 * @return Number of bytes read (may be less than @em siz)
 *
 * @see dime_ringbuffer_read
 * @see dime_ringbuffer_discard
 */
size_t dime_ringbuffer_peek(const dime_ringbuffer_t *ring,
                            void *buf,
                            size_t siz);

/**
 * @brief Advance a certain number of bytes in the ring buffer
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 * @param siz Number of bytes to discard
 *
 * @return Number of bytes discarded (may be less than @em siz)
 *
 * @see dime_ringbuffer_peek
 */
size_t dime_ringbuffer_discard(dime_ringbuffer_t *ring, size_t siz);

/**
 * @brief Get the number of bytes in the ring buffer
 *
 * @param ring Pointer to a @c dime_ringbuffer_t struct
 *
 * @return Number of bytes in the ring buffer
 */
size_t dime_ringbuffer_len(const dime_ringbuffer_t *ring);

#ifdef __cplusplus
}
#endif

#endif
