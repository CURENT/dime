#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include "ringbuffer.h"
#include "socket.h"

struct dime_header {
    int8_t magic[4];
    uint32_t jsondata_len;
    uint32_t bindata_len;
};

static const size_t SENDBUFLEN = 200000000;
static const size_t RECVBUFLEN = 200000000;

int dime_socket_init(dime_socket_t *sock, int fd) {
    sock->fd = fd;


    if (dime_ringbuffer_init(&sock->rbuf) < 0) {
        return -1;
    }

    if (dime_ringbuffer_init(&sock->wbuf) < 0) {
        dime_ringbuffer_destroy(&sock->rbuf);

        return -1;
    }

    return 0;
}

void dime_socket_destroy(dime_socket_t *sock) {
    dime_ringbuffer_destroy(&sock->rbuf);
    dime_ringbuffer_destroy(&sock->wbuf);
}

ssize_t dime_socket_push(dime_socket_t *sock, const json_t *jsondata, const void *bindata, size_t bindata_len) {
    char *jsonstr = json_dumps(jsondata, JSON_COMPACT);
    if (jsonstr == NULL) {
        return -1;
    }

    ssize_t ret = dime_socket_push_str(sock, jsonstr, bindata, bindata_len);

    free(jsonstr);
    return ret;
}

ssize_t dime_socket_push_str(dime_socket_t *sock, const char *jsonstr, const void *bindata, size_t bindata_len) {
    struct dime_header hdr;

    memcpy(hdr.magic, "DiME", 4);

    size_t jsondata_len = strlen(jsonstr);

    hdr.jsondata_len = htonl(jsondata_len);
    hdr.bindata_len = htonl(bindata_len);

    if (dime_ringbuffer_write(&sock->wbuf, &hdr, 12) < 12 ||
        dime_ringbuffer_write(&sock->wbuf, jsonstr, jsondata_len) < jsondata_len ||
        dime_ringbuffer_write(&sock->wbuf, bindata, bindata_len) < bindata_len) {
        return -1;
    }

    return 12 + jsondata_len + bindata_len;
}

ssize_t dime_socket_pop(dime_socket_t *sock, json_t **jsondata, void **bindata, size_t *bindata_len) {
    struct dime_header hdr;

    if (dime_ringbuffer_peek(&sock->rbuf, &hdr, 12) == 12) {
        if (memcmp(&hdr, "DiME", 4) != 0) {
            return -1;
        }

        hdr.jsondata_len = ntohl(hdr.jsondata_len);
        hdr.bindata_len = ntohl(hdr.bindata_len);

        size_t msgsiz = 12 + hdr.jsondata_len + hdr.bindata_len;

        void *buf = malloc(msgsiz);
        if (buf == NULL) {
            return -1;
        }

        if (dime_ringbuffer_peek(&sock->rbuf, buf, msgsiz) == msgsiz) {
            json_error_t jsonerr;
            json_t *jsondata_p = json_loadb((char *)buf + 12, hdr.jsondata_len, 0, &jsonerr);
            if (jsondata_p == NULL) {
                free(buf);

                return -1;
            }

            void *bindata_p = malloc(hdr.bindata_len);

            if (bindata_p == NULL) {
                json_decref(jsondata_p);
                free(buf);

                return -1;
            }

            memcpy(bindata_p, (unsigned char *)buf + 12 + hdr.jsondata_len, hdr.bindata_len);

            *jsondata = jsondata_p;
            *bindata = bindata_p;
            *bindata_len = hdr.bindata_len;

            dime_ringbuffer_discard(&sock->rbuf, msgsiz);

            free(buf);
            return msgsiz;
        }

        free(buf);
    }

    return 0;
}

ssize_t dime_socket_sendpartial(dime_socket_t *sock) {
    void *buf = malloc(SENDBUFLEN);
    if (buf == NULL) {
        return -1;
    }

    size_t nread = dime_ringbuffer_peek(&sock->wbuf, buf, SENDBUFLEN);
    ssize_t nsent = send(sock->fd, buf, nread, 0);

    if (nsent < 0) {
        free(buf);
        return -1;
    }

    dime_ringbuffer_discard(&sock->wbuf, nsent);
    free(buf);

    return nsent;
}

ssize_t dime_socket_recvpartial(dime_socket_t *sock) {
    void *buf = malloc(RECVBUFLEN);
    if (buf == NULL) {
        return -1;
    }

    ssize_t nrecvd = recv(sock->fd, buf, RECVBUFLEN, 0);

    if (nrecvd < 0) {
        free(buf);
        return -1;
    }

    if (dime_ringbuffer_write(&sock->rbuf, buf, nrecvd) < 0) {
        free(buf);
        return -1;
    }

    free(buf);

    return nrecvd;
}

int dime_socket_fd(const dime_socket_t *sock) {
    return sock->fd;
}

size_t dime_socket_sendlen(const dime_socket_t *sock) {
    return dime_ringbuffer_len(&sock->wbuf);
}

size_t dime_socket_recvlen(const dime_socket_t *sock) {
    return dime_ringbuffer_len(&sock->rbuf);
}
