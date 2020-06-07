#include <stdint.h>

#include <jansson.h>
#include "deque.h"
#include "server.h"
#include "socket.h"

#ifndef __DIME_client_H
#define __DIME_client_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int refs;

    json_t *jsondata;
    size_t bindata_len;
    unsigned char bindata[];
} dime_rcmessage_t;

typedef struct {
    int fd;
    char *name;

    dime_socket_t sock;
    dime_deque_t queue;
} dime_client_t;

int dime_client_init(dime_client_t *clnt, int fd);

void dime_client_destroy(dime_client_t *clnt);

int dime_client_join(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

int dime_client_leave(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

int dime_client_send(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

int dime_client_broadcast(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

int dime_client_sync(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

int dime_client_devices(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void *bindata, size_t bindata_len);

#ifdef __cplusplus
}
#endif

#endif
