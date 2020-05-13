
#include <stddef.h>
#include <sys/types.h>

#ifndef __DIME_socket_H
#define __DIME_socket_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dime_socket dime_socket_t;

dime_socket_t *dime_socket_new(int fd);

void dime_socket_free(dime_socket_t *sock);

int dime_socket_push(dime_socket_t *conn, const json_t *jsondata, const void *bindata, size_t bindata_len);

ssize_t dime_socket_sendpartial(dime_socket_t *conn);

ssize_t dime_socket_recvpartial(dime_socket_t *conn, json_t **jsondata, void **bindata, size_t *bindata_len);

int dime_socket_fd(const dime_socket_t *conn);

size_t dime_socket_sendsize(const dime_socket_t *conn);

#ifdef __cplusplus
}
#endif

#endif
