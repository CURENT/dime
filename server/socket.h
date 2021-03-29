/*
 * socket.h - Asynchronous DiME socket
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
 * @file socket.h
 * @brief Asynchronous DiME socket
 * @author Nicholas West
 * @date 2020
 *
 * A wrapper for a stream-oriented socket to send and receive DiME
 * messages on. A DiME message consists of the following data:
 * - A 4-byte magic value ("DiME" in ASCII)
 * - A 4 byte big-endian value for the size in bytes of the JSON portion
 *   of the data
 * - A 4 byte big-endian value for the size in bytes of the binary
 *   portion of the data
 * - The JSON portion of the data
 * - The binary portion of the data
 *
 * Both input and output from the underlying file descriptor is buffered
 * via @link dime_ringbuffer_t @endlink ring buffers. The @c read and
 * @c write syscalls and parsing/building data from the buffers is
 * decoupled; this enables the socket to queue messages on the socket
 * without blocking for them to be written, and allows it to perform a
 * read if input is detected via @c select or @c poll without waiting
 * for a full message to be received.

 */

#include <stddef.h>
#include <sys/types.h>

#include <jansson.h>
#include <openssl/ssl.h>
#include <zlib.h>
#include "ringbuffer.h"

#ifndef __DIME_socket_H
#define __DIME_socket_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Asynchronous DiME socket
 *
 * Should be treated as opaque; use relevant methods to send and receive
 * messages on the socket.
 *
 * @see dime_socket_init
 * @see dime_socket_destroy
 * @see dime_socket_push
 * @see dime_socket_push_str
 * @see dime_socket_pop
 * @see dime_socket_sendpartial
 * @see dime_socket_recvpartial
 * @see dime_socket_fd
 * @see dime_socket_sendlen
 * @see dime_socket_recvlen
 */
typedef struct {
    int fd; /** File descriptor */

    dime_ringbuffer_t rbuf; /** Inbuffer */
    dime_ringbuffer_t wbuf; /** Outbuffer */

    struct {
        int enabled;
        SSL *ctx;
    } tls;

    struct {
        int enabled;
        dime_ringbuffer_t rbuf;
    } ws;

    struct {
        int enabled;
        z_stream ctx;
        dime_ringbuffer_t rbuf;
    } zlib;

    char err[81]; /** Error string */
} dime_socket_t;

/**
 * @brief Initialize a new socket
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 * @param fd File descriptor to send/receive on
 * @param websocket non-zero for a WebSocket connection, zero otherwise
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_socket_destroy
 */
int dime_socket_init(dime_socket_t *sock, int fd);

/**
 * @brief Free resources used by a socket
 *
 * By default, this function closes the file descriptor that was used to
 * initialize the socket. Pass a file descriptor created with @c dup if
 * this behavior is undesirable.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @see dime_socket_init
 */
void dime_socket_destroy(dime_socket_t *sock);

/**
 * @brief Enable TLS encryption on the socket
 *

 * Performs a TLS handshake on the underlying socket. Subsequent reads/writes from the socket will be encrypted anddecrypted via TLS.

 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 * @param ctx OpenSSL context
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @todo Make this non-blocking
 * @see dime_socket_init_zlib
 */
int dime_socket_init_tls(dime_socket_t *sock, SSL_CTX *ctx);

/**
 * @brief Enable WebSocket protocol on the socket
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @todo Make this non-blocking
 */
int dime_socket_init_ws(dime_socket_t *sock);

/**
 * @brief Enable zlib compression on the socket
 *

 * Subsequent reads/writes on the socket will be (de)compressed via zlib.

 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @todo Make this non-blocking
 * @see dime_socket_init_tls
 */
int dime_socket_init_zlib(dime_socket_t *sock);

/**
 * @brief Adds a DiME message to the outbuffer
 *
 * Composes a DiME message from the JSON data in @em jsondata and the
 * binary data in @em bindata, and adds the message to the outbuffer.
 * The message is not sent directly; subsequent calls to
 * @link dime_socket_sendpartial @endlink will send the message at some
 * point in the future.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 * @param jsondata JSON portion of the message to send
 * @param bindata Binary portion of the message to send
 * @param bindata_len Length of binary data
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_socket_push_str
 * @see dime_socket_pop
 * @see dime_socket_sendpartial
 */
ssize_t dime_socket_push(dime_socket_t *sock,
                         const json_t *jsondata,
                         const void *bindata,
                         size_t bindata_len);

/**
 * @brief Adds a DiME message to the outbuffer (string version)
 *
 * Functions identically to @link dime_socket_push @endlink, but a
 * pre-encoded JSON string is provided instead of a @em json_t handle.
 * Useful for saving computational power if the same message is sent
 * over multiple sockets.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 * @param jsonstr JSON portion of the message to send, as a
 * NUL-terminated string
 * @param bindata Binary portion of the message to send
 * @param bindata_len Length of binary data
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_socket_push
 */
ssize_t dime_socket_push_str(dime_socket_t *sock,
                             const char *jsonstr,
                             const void *bindata,
                             size_t bindata_len);

/**
 * @brief Attempts to get a DiME message from the inbuffer
 *
 * Attempts to construct a DiME message from the data in the inbuffer.
 * If there is enough data to construct a message, @em jsondata, @em
 * bindata, and @em bindata_len are updated with the corresponding data
 * read from the message. If a message is received, @em jsondata and @em
 * bindata should be freed with @c json_decref and @c free,
 * respectively, once they are no longer needed.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 * @param jsondata Pointer to the JSON portion of the message received
 * @param bindata Pointer to the binary portion of the message received
 * @param bindata_len Pointer to the length of the binary data
 *
 * @return A positive value on success, zero if there is no complete
 * message in the inbuffer, or a negative value on failure
 *
 * @see dime_socket_push
 * @see dime_socket_recvpartial
 */
ssize_t dime_socket_pop(dime_socket_t *sock,
                        json_t **jsondata,
                        void **bindata,
                        size_t *bindata_len);

/**
 * @brief Sends data in the outbuffer
 *
 * Sends any data previously written to the outbuffer by
 * @link dime_socket_push @endlink. Does not attempt to send all of the
 * data, so multiple calls may be necessary if that is desired.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return Number of bytes sent on success, or a negative value on
 * failure
 *
 * @see dime_socket_push
 * @see dime_socket_recvpartial
 */
ssize_t dime_socket_sendpartial(dime_socket_t *sock);

/**
 * @brief Recieves data and writes it to the inbuffer
 *
 * Attempts to receive a given amount of data. If data is received, it
 * is written to the inbuffer and can be interpreted by a later call to
 * @link dime_socket_pop @endlink.
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return Number of bytes received on success, or a negative value on
 * failure
 *
 * @see dime_socket_pop
 * @see dime_socket_sendpartial
 */
ssize_t dime_socket_recvpartial(dime_socket_t *sock);

/**
 * @brief Get the file descriptor of the socket
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return File descriptor
 */
int dime_socket_fd(const dime_socket_t *sock);

/**
 * @brief Get the number of bytes in the outbuffer of the socket
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return Number of bytes in the outbuffer
 */
size_t dime_socket_sendlen(const dime_socket_t *sock);

/**
 * @brief Get the number of bytes in the inbuffer of the socket
 *
 * @param sock Pointer to a @link dime_socket_t @endlink struct
 *
 * @return Number of bytes in the inbuffer
 */
size_t dime_socket_recvlen(const dime_socket_t *sock);

#ifdef __cplusplus
}
#endif

#endif
