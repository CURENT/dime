#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <jansson.h>
#include <openssl/ssl.h>
#include "ringbuffer.h"
#include "socket.h"

typedef struct {
    int8_t magic[4];
    uint32_t jsondata_len;
    uint32_t bindata_len;
} dime_header_t;

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

    sock->tls.enabled = 0;
    sock->ws.enabled = 0;
    sock->zlib.enabled = 0;

    return 0;
}

void dime_socket_destroy(dime_socket_t *sock) {
    dime_ringbuffer_destroy(&sock->rbuf);
    dime_ringbuffer_destroy(&sock->wbuf);

    if (sock->tls.enabled) {
        SSL_shutdown(sock->tls.ctx);
        SSL_free(sock->tls.ctx);
    }

    if (sock->ws.enabled) {
        dime_ringbuffer_destroy(&sock->ws.rbuf);
    }

    if (sock->zlib.enabled) {
        dime_ringbuffer_destroy(&sock->zlib.rbuf);
        deflateEnd(&sock->zlib.ctx);
    }

    shutdown(sock->fd, SHUT_RDWR);
    close(sock->fd);
}

int dime_socket_init_ws(dime_socket_t *sock) {
    int flags = fcntl(sock->fd, F_GETFL, 0);
    if (flags < 0) {
        printf("%d\n", __LINE__); return -1;
    }

    if ((flags & ~O_NONBLOCK) != flags && fcntl(sock->fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        printf("%d\n", __LINE__); return -1;
    }

    char *http_hdr;
    size_t http_len, http_cap;

    http_len = 0;
    http_cap = 500;
    http_hdr = malloc(http_cap);

    if (http_hdr == NULL) {
        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    do {
        ssize_t nrecvd = recv(sock->fd, http_hdr + http_len, http_cap - http_len, 0);

        if (nrecvd < 0) {
            free(http_hdr);

            if ((flags & ~O_NONBLOCK) != flags) {
                fcntl(sock->fd, F_SETFL, flags);
            }

            printf("%d\n", __LINE__); return -1;
        }

        http_len += nrecvd;

        if (http_len >= http_cap) {
            http_cap = (http_cap * 3) / 2;
            char *nbuf = realloc(http_hdr, http_cap);

            if (nbuf == NULL) {
                free(http_hdr);

                if ((flags & ~O_NONBLOCK) != flags) {
                    fcntl(sock->fd, F_SETFL, flags);
                }

                printf("%d\n", __LINE__); return -1;
            }

            http_hdr = nbuf;
        }

        http_hdr[http_len] = '\0';
    } while (strstr(http_hdr, "\r\n\r\n") == NULL);

    char *line, *saveptr;

    line = strtok_r(http_hdr, "\r\n", &saveptr);

    char method[8];
    int major, minor;

    if (sscanf(line, "%7s %*s HTTP/%d.%d", method, &major, &minor) != 3) {
        free(http_hdr);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    if (strcmp(method, "GET") != 0) {
        free(http_hdr);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    if (major * 10 + minor < 11) {
        free(http_hdr);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    char *connection, *upgrade, *sec_ws_key, *sec_ws_version;

    connection = upgrade = sec_ws_key = sec_ws_version = NULL;

    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
        char *delimiter = strstr(line, ": ");

        if (delimiter == NULL) {
            free(http_hdr);

            if ((flags & ~O_NONBLOCK) != flags) {
                fcntl(sock->fd, F_SETFL, flags);
            }

            printf("%d\n", __LINE__); return -1;
        }

        char *key, *val;

        delimiter[0] = '\0';
        key = line;
        val = delimiter + 2;

        if (strcmp(key, "Connection") == 0) {
            connection = val;
        } else if (strcmp(key, "Upgrade") == 0) {
            upgrade = val;
        } else if (strcmp(key, "Sec-WebSocket-Key") == 0) {
            sec_ws_key = val;
        } else if (strcmp(key, "Sec-WebSocket-Version") == 0) {
            sec_ws_version = val;
        }
    }

    if (connection == NULL || strstr(connection, "Upgrade") == NULL ||
        upgrade == NULL || strcmp(upgrade, "websocket") != 0 ||
        sec_ws_key == NULL || strlen(sec_ws_key) == 0 ||
        sec_ws_version == NULL || strcmp(sec_ws_version, "13") != 0) {

        assert(connection != NULL);
        assert(strstr(connection, "Upgrade") != NULL);
        assert(upgrade != NULL);
        assert(strcmp(upgrade, "websocket") != 0);
        assert(sec_ws_key != NULL);
        assert(strlen(sec_ws_key) != 0);
        assert(sec_ws_version != NULL);
        assert(strcmp(sec_ws_version, "13") == 0);

        free(http_hdr);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    SHA_CTX sha1;
    unsigned char sha1sum[20];
    char b64_sha1sum[29];

    SHA1_Init(&sha1);

    SHA1_Update(&sha1, sec_ws_key, strlen(sec_ws_key));
    SHA1_Update(&sha1, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

    SHA1_Final(sha1sum, &sha1);

    EVP_EncodeBlock((unsigned char *)b64_sha1sum, sha1sum, 20);

    free(http_hdr);

    char response[200];

    int response_len = snprintf(response, sizeof(response),
                                "HTTP/%d.%d 101 Switching Protocols\r\n"
                                "Connection: Upgrade\r\n"
                                "Upgrade: websocket\r\n"
                                "Sec-WebSocket-Accept: %s\r\n\r\n",
                                major, minor, b64_sha1sum);

    assert(response_len >= 0 && response_len < sizeof(response));

    if (send(sock->fd, response, response_len, 0) < 0) {
        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        printf("%d\n", __LINE__); return -1;
    }

    /* Reset the original socket flags */
    if ((flags & ~O_NONBLOCK) != flags && fcntl(sock->fd, F_SETFL, flags) < 0) {
        printf("%d\n", __LINE__); return -1;
    }

    sock->ws.enabled = 1;

    if (dime_ringbuffer_init(&sock->ws.rbuf) < 0) {
        printf("%d\n", __LINE__); return -1;
    }

    return 0;
}

int dime_socket_init_tls(dime_socket_t *sock, SSL_CTX *tlsctx) {
    /* Ensure the underlying socket is blocking for the TLS handshake */
    int flags = fcntl(sock->fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }

    if ((flags & ~O_NONBLOCK) != flags && fcntl(sock->fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        return -1;
    }

    assert(dime_ringbuffer_len(&sock->rbuf) == 0);

    while (dime_ringbuffer_len(&sock->wbuf) > 0) {
        if (dime_socket_sendpartial(sock) < 0) {
            if ((flags & ~O_NONBLOCK) != flags) {
                fcntl(sock->fd, F_SETFL, flags);
            }

            return -1;
        }
    }

    sock->tls.ctx = SSL_new(tlsctx);
    if (sock->tls.ctx == NULL) {
        return -1;
    }

    SSL_set_mode(sock->tls.ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

    if (SSL_set_fd(sock->tls.ctx, sock->fd) <= 0) {
        SSL_free(sock->tls.ctx);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        return -1;
    }

    if (SSL_accept(sock->tls.ctx) <= 0) {
        SSL_free(sock->tls.ctx);

        if ((flags & ~O_NONBLOCK) != flags) {
            fcntl(sock->fd, F_SETFL, flags);
        }

        return -1;
    }

    /* Reset the original socket flags */
    if ((flags & ~O_NONBLOCK) != flags && fcntl(sock->fd, F_SETFL, flags) < 0) {
        return -1;
    }

    sock->tls.enabled = 1;

    return 0;
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
    dime_header_t hdr;

    memcpy(hdr.magic, "DiME", 4);

    size_t ws_len = 0;
    size_t jsondata_len = strlen(jsonstr);

    hdr.jsondata_len = htonl(jsondata_len);
    hdr.bindata_len = htonl(bindata_len);

    if (sock->ws.enabled) {
        uint8_t ws_hdr[10];
        ws_hdr[0] = 0x82;

        size_t payload_len = 12 + jsondata_len + bindata_len;

        if (payload_len < 126) {
            ws_hdr[1] = payload_len;

            ws_len = 2;
        } else if (payload_len < (1 << 16)) {
            ws_hdr[1] = 126;
            ws_hdr[2] = (payload_len >> 8) & 0xFF;
            ws_hdr[3] = payload_len & 0xFF;

            ws_len = 4;
        } else {
            assert((payload_len & (1ull << 63)) == 0);

            ws_hdr[1] = 127;
            ws_hdr[2] = (payload_len >> 56) & 0xFF;
            ws_hdr[3] = (payload_len >> 48) & 0xFF;
            ws_hdr[4] = (payload_len >> 40) & 0xFF;
            ws_hdr[5] = (payload_len >> 32) & 0xFF;
            ws_hdr[6] = (payload_len >> 24) & 0xFF;
            ws_hdr[7] = (payload_len >> 16) & 0xFF;
            ws_hdr[8] = (payload_len >> 8) & 0xFF;
            ws_hdr[9] = payload_len & 0xFF;

            ws_len = 10;
        }

        if (dime_ringbuffer_write(&sock->wbuf, ws_hdr, ws_len) < ws_len) {
            return -1;
        }
    }

    if (dime_ringbuffer_write(&sock->wbuf, &hdr, 12) < 12 ||
        dime_ringbuffer_write(&sock->wbuf, jsonstr, jsondata_len) < jsondata_len ||
        dime_ringbuffer_write(&sock->wbuf, bindata, bindata_len) < bindata_len) {

        return -1;
    }

    return 12 + ws_len + jsondata_len + bindata_len;
}

ssize_t dime_socket_pop(dime_socket_t *sock, json_t **jsondata, void **bindata, size_t *bindata_len) {
    if (sock->ws.enabled) {
        while (1) {
            uint8_t ws_hdr[14], mask[4];
            size_t hdr_len, frame_len;

            size_t nread = dime_ringbuffer_peek(&sock->ws.rbuf, ws_hdr, 14);

            if ((ws_hdr[1] & ~0x80) < 126 && nread >= 6) {
                hdr_len = 6;
                frame_len = ws_hdr[1] & ~0x80;

                mask[0] = ws_hdr[2];
                mask[1] = ws_hdr[3];
                mask[2] = ws_hdr[4];
                mask[3] = ws_hdr[5];
            } else if ((ws_hdr[1] & ~0x80) == 126 && nread >= 8) {
                hdr_len = 8;
                frame_len = ((size_t)ws_hdr[2] << 8) | ws_hdr[3];

                mask[0] = ws_hdr[4];
                mask[1] = ws_hdr[5];
                mask[2] = ws_hdr[6];
                mask[3] = ws_hdr[7];
            } else if ((ws_hdr[1] & ~0x80) == 127 && nread == 14) {
                hdr_len = 14;
                frame_len = ((size_t)ws_hdr[2] << 56) |
                            ((size_t)ws_hdr[3] << 48) |
                            ((size_t)ws_hdr[4] << 40) |
                            ((size_t)ws_hdr[5] << 32) |
                            ((size_t)ws_hdr[6] << 24) |
                            ((size_t)ws_hdr[7] << 16) |
                            ((size_t)ws_hdr[8] << 8) |
                            ws_hdr[9];

                mask[0] = ws_hdr[10];
                mask[1] = ws_hdr[11];
                mask[2] = ws_hdr[12];
                mask[3] = ws_hdr[13];
            } else {
                break;
            }

            if ((ws_hdr[1] & 0x80) == 0) {
                return -1;
            }

            size_t msgsiz = hdr_len + frame_len;
            unsigned char *msg = malloc(msgsiz);
            if (msg == NULL) {
                return -1;
            }

            if (dime_ringbuffer_peek(&sock->ws.rbuf, msg, msgsiz) == msgsiz) {
                unsigned char *frame = msg + hdr_len;

                for (size_t i = 0; i < frame_len; i++) {
                    frame[i] ^= mask[i & 3];
                }

                if (dime_ringbuffer_write(&sock->rbuf, frame, frame_len) < 0) {
                    free(msg);

                    return -1;
                }

                dime_ringbuffer_discard(&sock->ws.rbuf, msgsiz);
                free(msg);
            } else {
                free(msg);
                break;
            }
        }
    }

    dime_header_t hdr;

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
    ssize_t nsent;

    if (sock->tls.enabled) {
        nsent = SSL_write(sock->tls.ctx, buf, nread);
    } else {
        nsent = send(sock->fd, buf, nread, 0);
    }

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

    ssize_t nrecvd;

    if (sock->tls.enabled) {
        nrecvd = SSL_read(sock->tls.ctx, buf, RECVBUFLEN);
    } else {
        nrecvd = recv(sock->fd, buf, RECVBUFLEN, 0);
    }

    if (nrecvd < 0) {
        free(buf);

        return -1;
    }

    dime_ringbuffer_t *rbuf;

    if (sock->ws.enabled) {
        rbuf = &sock->ws.rbuf;
    } else if (sock->zlib.enabled) {
        rbuf = &sock->zlib.rbuf;
    } else {
        rbuf = &sock->rbuf;
    }

    if (dime_ringbuffer_write(rbuf, buf, nrecvd) < 0) {
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
