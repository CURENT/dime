/*
 * client.h - Client-related subroutines
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
 * Contains data structures and subroutines intended to manage a
 * client's state. Each valid command that can be sent by clients has a
 * corresponding function in this file to handle it.
 */

#include <stdint.h>
#include <sys/socket.h>

#include <jansson.h>
#include "deque.h"
#include "server.h"
#include "socket.h"

#ifndef __DIME_client_H
#define __DIME_client_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Client's state
 *
 * Contains all data relevant to a single client's server-side state.
 * This includes a @link dime_socket_t @endlink and a
 * @link dime_deque_t @endlink queue of @link dime_rcmessage_t @endlink
 * messages that act as a secondary buffer. "send" and "broadcast"
 * commands from other clients add messages to the queue, while "sync"
 * commands flush the queue to the socket.
 *
 * @see dime_client_init
 * @see dime_client_destroy
 * @see dime_client_join
 * @see dime_client_leave
 * @see dime_client_send
 * @see dime_client_broadcast
 * @see dime_client_sync
 * @see dime_client_wait
 * @see dime_client_devices
 */
typedef struct __dime_client dime_client_t;

/**
 * @brief Reference-counted message
 *
 * Record that contains a single DiME message and a count of how many
 * references it has in memory. Note that there are no functions
 * introduced to manage this struct; the reference count is managed
 * directly by the functions in this file, and when the reference count
 * reaches zero, it is deallocated manually by said functions.
 */
typedef struct {
    unsigned int refs; /** Reference count */

    char *jsondata;     /** JSON portion of the message as a string */
    void *bindata;      /** Binary portion of the message */
    size_t bindata_len; /** Length of binary portion of the message */
} dime_rcmessage_t;

/**
 * @brief Group of clients
 *
 * Record that contains a list of clients that all share a named group.
 */
typedef struct {
    char *name; /** Group name */

    dime_client_t **clnts; /** Array of clients */
    size_t clnts_len;      /** Length of client array */
    size_t clnts_cap;      /** Capacity of client array */
} dime_group_t;

struct __dime_client {
    int fd;      /** File descriptor */
    int waiting; /** Whether or not this client is waiting for a new message */

    char *addr; /** Address of connection, as a human-readable string */

    dime_group_t **groups; /** Array of associated groups */
    size_t groups_len;     /** Length of groups */
    size_t groups_cap;     /** Capacity of groups */

    dime_socket_t sock; /** DiME socket */
    dime_deque_t queue; /** Queue of reference-counted messages */
};

/**
 * @brief Initialize a new client
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param fd File descriptor to send/receive on
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_destroy
 */
int dime_client_init(dime_client_t *clnt, int fd, const struct sockaddr *addr);

/**
 * @brief Free resources used by a client
 *
 * By default, this function closes the file descriptor that was used to
 * initialize the client. Pass a file descriptor created with @c dup if
 * this behavior is undesirable.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 *
 * @see dime_client_init
 */
void dime_client_destroy(dime_client_t *clnt);

/**
 * @brief Handle a "register" command
 *
 * The "register" command mostly does housekeeping w.r.t. the
 * serialization method.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @todo Remove this
 */
int dime_client_register(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "join" command
 *
 * The "join" command instructs the server to add the client @em clnt to
 * the group specified in the JSON field @c name.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_leave
 */
int dime_client_join(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "leave" command
 *
 * The "leave" command instructs the server to remove the client @em
 * clnt from the group specified in the JSON field @c name.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_join
 * @todo Implement this
 */
int dime_client_leave(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "send" command
 *
 * The "send" command instructs the server to relay the message to all
 * clients in the group specified in the JSON field @c name.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_broadcast
 * @see dime_client_sync
 * @see dime_client_wait
 */
int dime_client_send(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "broadcast" command
 *
 * The "broadcast" command instructs the server to relay the message to
 * all other clients.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_send
 * @see dime_client_sync
 * @see dime_client_wait
 */
int dime_client_broadcast(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "sync" command
 *

 * The "sync" command instructs the server to send the client @em clnt
 * the messages that have been relayed by other clients. The JSON field
 * @c n may optionally specify a limit on the number of messages to
 * download.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_send
 * @see dime_client_broadcast
 */
int dime_client_sync(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "wait" command
 *
 * The "sync" command instructs the server to send the client @em clnt a 
 * DiME message as soon as another client has queued a message via a 
 * "send" or "broadcast" command. This effectively blocks the client's 
 * thread of execution until at least one message can be retrieved.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 *
 * @see dime_client_send
 * @see dime_client_broadcast
 */
int dime_client_wait(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

/**
 * @brief Handle a "devices" command
 *
 * The "devices" command instructs the server to send the client a list
 * of the groups with active clients in them.
 *
 * @param clnt Pointer to a @link dime_client_t @endlink struct
 * @param srv Pointer to the @link dime_server_t @endlink struct from
 * which the client connection was accepted
 * @param jsondata JSON portion of the message
 * @param pbindata Binary portion of the message
 * @param bindata_len Length of binary portion of the message
 *
 * @return A nonnegative value on success, or a negative value on
 * failure
 */
int dime_client_devices(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len);

#ifdef __cplusplus
}
#endif

#endif

