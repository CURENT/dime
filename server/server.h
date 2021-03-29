/*
 * server.h - Server-related subroutines
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
 * @file client.h
 * @brief Client-related subroutines
 * @author Nicholas West
 * @date 2020
 *
 * Contains data structures and subroutines intended to manage the
 * server's state. The code creates a socket that listens for
 * connections based on configuration variables set in the server
 * struct, and creates a @link dime_client_t @endlink struct for each
 * incoming connection. It then uses an event loop with @c poll to handle
 * incoming reads and outgoing writes.
 *
 * @todo This could be global data, assuming we only run one server per process
 */

#include <stdint.h>

#include <openssl/ssl.h>

#include "table.h"

#ifndef __DIME_server_H
#define __DIME_server_H

#ifdef __cplusplus
extern "C" {
#endif

enum dime_serialization {
    DIME_NO_SERIALIZATION,
    DIME_MATLAB,
    DIME_PICKLE,
    DIME_DIMEB,
    DIME_JSON
};

typedef struct {
    int fd;
    int protocol;
} dime_server_fd_t;

enum dime_protocol {
    DIME_UNIX,
    DIME_TCP,
    DIME_WS
};

/**
 * @brief Client's state
 *
 * Contains all data relevant to a single server's state. The configuration variables should be set before calling @link dime_server_init @endlink.
 */
typedef struct {
    char err[81]; /** Error string */

    unsigned int daemon : 1; /** Daemon flag */
    unsigned int tls : 1;    /** TLS flag */
    unsigned int zlib : 1;   /** zlib flag */
    unsigned int ws : 1;     /** WebSocket flag */
    char : 0;

    dime_server_fd_t *fds;
    size_t fds_len;
    size_t fds_cap;

    char **pathnames;
    size_t pathnames_len;
    size_t pathnames_cap;

    const char *certname;    /** Certificate pathname (if using TLS) */
    const char *privkeyname; /** Private key pathname (if using TLS) */
    const char *socketname;  /** Socket pathname (if using Unix socket) */
    uint16_t port;           /** Port (if using TCP socket) */

    unsigned int verbosity; /** Verbosity level */
    unsigned int threads;   /** Number of worker threads */
    int protocol;           /** Protocol to use */
    int serialization;      /** Serialization method */

    int fd;                 /** File descriptor */
    dime_table_t fd2clnt;   /** File descriptor-to-client translation table */
    dime_table_t name2clnt; /** Name-to-client translation table */
    SSL_CTX *tlsctx;        /** OpenSSL context */
} dime_server_t;

/**
 * @brief Initialize a new server
 *
 * Configuration variables in @em srv, such as @c pathname, @c port, etc. should be set before calling this function.
 *
 * @param srv Pointer to a @link dime_server_t @endlink struct
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_server_destroy
 */
int dime_server_init(dime_server_t *srv);

/**
 * @brief Free resources used by a server
 *
 * @param srv Pointer to a @link dime_server_t @endlink struct
 *
 * @see dime_server_init
 */
void dime_server_destroy(dime_server_t *srv);

int dime_server_add(dime_server_t *srv, int protocol, ...);

/**
 * @brief Run the event loop for the server
 *
 * This function does not return until either the process is sent a @c SIGINT or @c SIGTERM signal, or it encounters an irrecoverable error.
 *
 * @param srv Pointer to a @link dime_server_t @endlink struct
 *
 * @return A nonnegative value upon receiving a @c SIGINT or @c SIGTERM signal, or a negative value on failure
 */
int dime_server_loop(dime_server_t *srv);

#ifdef __cplusplus
}
#endif

#endif
