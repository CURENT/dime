#include <stdint.h>

#ifndef __DIME_server_H
#define __DIME_server_H

#ifdef __cplusplus
extern "C" {
#endif

enum dime_protocol {
    DIME_UNIX,
    DIME_TCP
    /*
    DIME_TCP_IPV6,
    DIME_SCTP,
    DIME_SCTP_IPV6
    */
};

typedef struct dime_server dime_server_t;

dime_server_t *dime_server_new(int protocol, const char *socketfile, uint16_t port);

void dime_server_free(dime_server_t *srv);

int dime_server_loop(dime_server_t *srv);

#ifdef __cplusplus
}
#endif

#endif
