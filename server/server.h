#include <stdint.h>

#include "table.h"

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

enum dime_serialization {
    DIME_NO_SERIALIZATION,
    DIME_MATLAB,
    DIME_PICKLE,
    DIME_DIMEB
};

typedef struct {
    int ipv6 : 1;
    int ssl : 1;
    int verbose : 1;
    char : 0;

    int protocol;
    const char *pathname;
    uint16_t port;

    char *err;

    int fd;
    dime_table_t fd2conn, name2conn;

    int serialization;
} dime_server_t;

int dime_server_init(dime_server_t *srv);

void dime_server_destroy(dime_server_t *srv);

int dime_server_loop(dime_server_t *srv);

#ifdef __cplusplus
}
#endif

#endif
