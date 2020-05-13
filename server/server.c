#include <stdint.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <jansson.h>

#include "server.h"
#include "table.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define FAIL_LOUDLY() { perror(__FILE__ ":" STRINGIZE(__LINE__)); abort(); }

struct dime_rcmessage {
    int refcount;

    json_t *jsondata;
    void *bindata;
    size_t bindata_len;
};

struct dime_rcmessage_node {
    struct dime_rcmessage *data;
    struct dime_rcmessage_node *next;
};

struct dime_connection {
    dime_socket_t *sock;

    int fd;
    char *name;

    struct dime_rcmessage_node *head, *tail;
};

struct dime_server {
    int fd;
    int protocol;

    dime_table_t *fd2conn, *name2conn;
};

static int cmp_fd(const void *a, const void *b) {
    return (*(const int *)b) - (*(const int *)a);
}

static uint64_t hash_fd(const void *a) {
    return (*(const int *)a) * 0x9E3779B97F4A7BB9;
}

static int cmp_name(const void *a, const void *b) {
    return strcmp(a, b);
}

static uint64_t hash_name(const void *a) {
    uint64_t y = 0xCBF29CE484222325;

    for (const char *s = a; *s != '\0'; s++) {
        y = (y ^ *s) * 1099511628211;
    }

    return y;
}

dime_server_t *dime_server_new(int protocol, const char *socketfile, uint16_t port) {
    dime_server_t *srv;

    srv = malloc(sizeof(dime_server_t));
    if (srv == NULL) {
        return NULL;
    }

    srv->protocol = protocol;

    srv->fd2conn = dime_table_new(cmp_fd, hash_fd);
    if (srv->fd2conn == NULL) {
        free(srv);

        return NULL;
    }

    srv->name2conn = dime_table_new(cmp_name, hash_name);
    if (srv->name2conn == NULL) {
        dime_table_free(srv->fd2conn);
        free(srv);

        return NULL;
    }

    union {
        struct sockaddr_in inet;
        struct sockaddr_un uds;
    } addr;
    size_t addrlen;
    int socktype, proto;

    memset(&addr, 0, sizeof(addr));

    switch (protocol) {
    case DIME_UNIX:
        addr.uds.sun_family = AF_UNIX;
        strncpy(addr.uds.sun_path, socketfile, sizeof(addr.uds.sun_path));
        addr.uds.sun_path[sizeof(addr.uds.sun_path) - 1] = '\0';

        socktype = AF_UNIX;
        proto = 0;
        addrlen = sizeof(struct sockaddr_un);

        break;

    case DIME_TCP:
        addr.inet.sin_family = AF_INET;
        addr.inet.sin_addr.s_addr = INADDR_ANY;
        addr.inet.sin_port = htons(port);

        socktype = AF_INET;
        proto = IPPROTO_TCP;
        addrlen = sizeof(struct sockaddr_in);

        break;

    default:
        dime_table_free(srv->name2conn);
        dime_table_free(srv->fd2conn);
        free(srv);
        return NULL;
    }

    srv->fd = socket(socktype, SOCK_STREAM, proto);
    if (srv->fd < 0) {
        dime_table_free(srv->name2conn);
        dime_table_free(srv->fd2conn);
        free(srv);

        return NULL;
    }

    if (bind(srv->fd, (struct sockaddr *)&addr, addrlen) < 0) {
        close(srv->fd);
        dime_table_free(srv->name2conn);
        dime_table_free(srv->fd2conn);
        free(srv);

        return NULL;
    }

    if (listen(srv->fd, 5) < 0) {
        close(srv->fd);
        dime_table_free(srv->name2conn);
        dime_table_free(srv->fd2conn);
        free(srv);

        return NULL;
    }

    return srv;
}

int dime_server_loop(dime_server_t *srv) {
    struct pollfd *pollfds;
    size_t pollfds_len, pollfds_cap;

    pollfds_cap = 16;
    pollfds = malloc(pollfds_cap * sizeof(struct pollfd));
    if (pollfds == NULL) {
        FAIL_LOUDLY();
    }

    pollfds[0].fd = srv->fd;
    pollfds[0].events = POLLIN;

    pollfds_len = 1;

    while (1) {
        printf("Polling... ");
        if (poll(pollfds, pollfds_len, -1) < 0) {
            FAIL_LOUDLY();
        }
        printf("Done!\n");

        if (pollfds[0].revents & POLLIN) {
            struct dime_connection *conn = malloc(sizeof(struct dime_connection));
            if (conn == NULL) {
                FAIL_LOUDLY();
            }

            conn->fd = accept(srv->fd, NULL, NULL);
            if (conn->fd < 0) {
                FAIL_LOUDLY();
            }

            printf("Opening socket %d\n", conn->fd);

            conn->sock = dime_socket_new(conn->fd);
            if (conn->sock == NULL) {
                FAIL_LOUDLY();
            }

            conn->name = NULL;
            conn->head = conn->tail = NULL;

            if (dime_table_insert(srv->fd2conn, &conn->fd, conn) < 0) {
                FAIL_LOUDLY();
            }

            pollfds[pollfds_len].fd = conn->fd;
            pollfds[pollfds_len].events = POLLIN;
            pollfds[pollfds_len].revents = 0;
            pollfds_len++;

            if (pollfds_len >= pollfds_cap) {
                size_t ncap = (3 * pollfds_cap) / 2;
                void *narr = realloc(pollfds, ncap * sizeof(struct pollfd));
                if (narr == NULL) {
                    FAIL_LOUDLY();
                }

                pollfds = narr;
                pollfds_cap = ncap;
            }
        }

        for (size_t i = 1; i < pollfds_len; i++) {
            struct dime_connection *conn = dime_table_search(srv->fd2conn, &pollfds[i].fd);

            if (pollfds[i].revents & POLLIN) {
                printf("Got POLLIN on socket %d\n", conn->fd);

                json_t *jsondata;
                void *bindata;
                size_t bindata_len;

                ssize_t n = dime_socket_recvpartial(conn->sock, &jsondata, &bindata, &bindata_len);

                if (n < 0) {
                    FAIL_LOUDLY();
                } else if (n > 0) {
                    const char *cmd;

                    if (json_unpack(jsondata, "{ss}", "command", &cmd) < 0) {
                        FAIL_LOUDLY();
                    }

                    puts(cmd);

                    if (strcmp(cmd, "register") == 0) {
                        if (conn->name != NULL) {
                            FAIL_LOUDLY();
                        }

                        const char *name;

                        if (json_unpack(jsondata, "{ss}", "name", &name) < 0) {
                            FAIL_LOUDLY();
                        }

                        conn->name = strdup(name);
                        if (conn->name == NULL) {
                            FAIL_LOUDLY();
                        }

                        if (dime_table_insert(srv->name2conn, conn->name, conn) < 0) {
                            FAIL_LOUDLY();
                        }

                        json_decref(jsondata);
                        free(bindata);

                        jsondata = json_pack("{si}", "status", 0);
                        if (jsondata == NULL) {
                            FAIL_LOUDLY();
                        }

                        dime_socket_push(conn->sock, jsondata, NULL, 0);

                        json_decref(jsondata);

                        pollfds[i].events |= POLLOUT;
                    } else if (strcmp(cmd, "send") == 0) {
                        const char *name;

                        if (json_unpack(jsondata, "{ss}", "name", &name) < 0) {
                            FAIL_LOUDLY();
                        }

                        struct dime_connection *other = dime_table_search(srv->name2conn, name);
                        if (other == NULL) {
                            FAIL_LOUDLY();
                        }

                        struct dime_rcmessage *msg = malloc(sizeof(struct dime_rcmessage));
                        if (msg == NULL) {
                            FAIL_LOUDLY();
                        }

                        msg->refcount = 1;
                        msg->jsondata = jsondata;
                        msg->bindata = bindata;
                        msg->bindata_len = bindata_len;

                        struct dime_rcmessage_node *node = malloc(sizeof(struct dime_rcmessage_node));
                        if (node == NULL) {
                            FAIL_LOUDLY();
                        }

                        node->data = msg;
                        node->next = NULL;

                        if (other->tail == NULL) {
                            other->head = node;
                        } else {
                            other->tail->next = node;
                        }

                        other->tail = node;
                    } else if (strcmp(cmd, "broadcast") == 0) {
                        /* TODO: Implement this */
                        FAIL_LOUDLY();
                    } else if (strcmp(cmd, "sync") == 0) {
                        json_decref(jsondata);
                        free(bindata);

                        struct dime_rcmessage_node *node = conn->head;

                        while (node != NULL) {
                            struct dime_rcmessage *msg = node->data;

                            if (dime_socket_push(conn->sock, msg->jsondata, msg->bindata, msg->bindata_len) < 0) {
                                FAIL_LOUDLY();
                            }

                            msg->refcount--;
                            if (msg->refcount <= 0) {
                                json_decref(msg->jsondata);
                                free(msg->bindata);
                                free(msg);
                            }

                            struct dime_rcmessage_node *next = node->next;
                            free(node);
                            node = next;
                        }

                        pollfds[i].events |= POLLOUT;
                    } else {
                        FAIL_LOUDLY();
                    }
                }
            }

            if (pollfds[i].revents & POLLOUT) {
                printf("Got POLLOUT on socket %d\n", conn->fd);

                if (dime_socket_sendpartial(conn->sock) < 0) {
                    FAIL_LOUDLY();
                }

                if (dime_socket_sendsize(conn->sock) == 0) {
                    pollfds[i].events &= ~POLLOUT;
                }
            }

            if (pollfds[i].revents & POLLHUP) {
                printf("Got POLLHUP on socket %d\n", conn->fd);

                if (dime_table_remove(srv->fd2conn, &pollfds[i].fd) == NULL) {
                    FAIL_LOUDLY();
                }

                if (conn->name != NULL) {
                    dime_table_remove(srv->name2conn, conn->name);
                    free(conn->name);
                }

                struct dime_rcmessage_node *node = conn->head;

                while (node != NULL) {
                    struct dime_rcmessage *msg = node->data;

                    msg->refcount--;
                    if (msg->refcount <= 0) {
                        json_decref(msg->jsondata);
                        free(msg->bindata);
                        free(msg);
                    }

                    struct dime_rcmessage_node *next = node->next;
                    free(node);
                    node = next;
                }

                printf("Closing socket %d\n", conn->fd);

                dime_socket_free(conn->sock);
                close(conn->fd);
                free(conn);

                pollfds_len--;
                pollfds[i] = pollfds[pollfds_len];
            }

            pollfds[i].revents = 0;
        }
    }
}

void dime_server_free(dime_server_t *srv) {
    shutdown(srv->fd, SHUT_RDWR);
    close(srv->fd);
    free(srv);
}
