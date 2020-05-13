#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "fifobuffer.h"
#include "socket.h"

struct dime_header {
    int8_t magic[4];
    uint32_t jsondata_len;
    uint32_t bindata_len;
};

static const size_t SENDBUFLEN = 1 << 16;
static const size_t RECVBUFLEN = 1 << 16;

struct dime_socket {
    int fd;
    dime_fifobuffer_t *rbuf, *wbuf;
};

dime_socket_t *dime_socket_new(int fd) {
    dime_socket_t *sock = malloc(sizeof(dime_socket_t));
    if (sock == NULL) {
        return NULL;
    }

    sock->fd = fd;

    sock->rbuf = dime_fifobuffer_new();
    if (sock->rbuf == NULL) {
        free(sock);

        return NULL;
    }

    sock->wbuf = dime_fifobuffer_new();
    if (sock->wbuf == NULL) {
        free(sock->rbuf);
        free(sock);

        return NULL;
    }

    return sock;
}

void dime_socket_free(dime_socket_t *sock) {
    dime_fifobuffer_free(sock->rbuf);
    dime_fifobuffer_free(sock->wbuf);
    free(sock);
}

int dime_socket_push(dime_socket_t *sock, const json_t *jsondata, const void *bindata, size_t bindata_len) {
    struct dime_header hdr;

    memcpy(hdr.magic, "DiME", 4);

    char *jsonstr = json_dumps(jsondata, JSON_COMPACT);
    if (jsonstr == NULL) {
        return -1;
    }

    size_t jsondata_len = strlen(jsonstr);

    hdr.jsondata_len = htonl(jsondata_len);
    hdr.bindata_len = htonl(bindata_len);

    if (dime_fifobuffer_write(sock->wbuf, &hdr, 12) < 12 ||
        dime_fifobuffer_write(sock->wbuf, jsonstr, jsondata_len) < jsondata_len ||
        dime_fifobuffer_write(sock->wbuf, bindata, bindata_len) < bindata_len) {

        free(jsonstr);
        return -1;
    }

    free(jsonstr);
    return 0;
}

ssize_t dime_socket_sendpartial(dime_socket_t *sock) {
    void *buf = malloc(SENDBUFLEN);
    if (buf == NULL) {
        return -1;
    }

    size_t nread = dime_fifobuffer_peek(sock->wbuf, buf, SENDBUFLEN);
    ssize_t nsent = send(sock->fd, buf, nread, 0);

    if (nsent < 0) {
        free(buf);
        return -1;
    }

    dime_fifobuffer_discard(sock->wbuf, nsent);
    free(buf);

    return nsent;
}

ssize_t dime_socket_recvpartial(dime_socket_t *sock, json_t **jsondata, void **bindata, size_t *bindata_len) {
    void *buf = malloc(RECVBUFLEN);
    if (buf == NULL) {
        return -1;
    }

    ssize_t nrecvd = recv(sock->fd, buf, RECVBUFLEN, 0);

    if (nrecvd < 0) {
        free(buf);
        return -1;
    }

    dime_fifobuffer_write(sock->rbuf, buf, nrecvd);

    /* Attempt to get a message from the read buffer */
    struct dime_header hdr;

    if (dime_fifobuffer_peek(sock->rbuf, &hdr, 12) == 12) {
        if (memcmp(hdr.magic, "DiME", 4) != 0) {
            return -1;
        }

        hdr.jsondata_len = ntohl(hdr.jsondata_len);
        hdr.bindata_len = ntohl(hdr.bindata_len);

        size_t msgsiz = 12 + hdr.jsondata_len + hdr.bindata_len;

        if (msgsiz > RECVBUFLEN) {
            free(buf);

            buf = malloc(msgsiz);
            if (buf == NULL) {
                return -1;
            }
        }

        if (dime_fifobuffer_peek(sock->rbuf, buf, msgsiz) == msgsiz) {
            json_error_t jsonerr;
            json_t *jsondata_p = json_loadb(buf + 12, hdr.jsondata_len, 0, &jsonerr);
            if (jsondata_p == NULL) {
                free(buf);

                return -1;
            }

            void *bindata_p;

            if (hdr.bindata_len > 0) {
                bindata_p = malloc(hdr.bindata_len);
                if (bindata_p == NULL) {
                    json_decref(jsondata_p);
                    free(buf);

                    return -1;
                }

                memcpy(bindata_p, buf + 12 + hdr.jsondata_len, hdr.bindata_len);
            } else {
                bindata_p = NULL;
            }

            *jsondata = jsondata_p;
            *bindata = bindata_p;
            *bindata_len = hdr.bindata_len;

            dime_fifobuffer_discard(sock->rbuf, msgsiz);

            free(buf);

            return msgsiz;
        }

    }

    free(buf);
    return 0;
}

int dime_socket_fd(const dime_socket_t *sock) {
    return sock->fd;
}

size_t dime_socket_sendsize(const dime_socket_t *sock) {
    return dime_fifobuffer_size(sock->wbuf);
}
