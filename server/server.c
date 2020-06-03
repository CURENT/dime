#include <stdint.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#include <jansson.h>

#include "server.h"
#include "table.h"
#include "deque.h"
#include "socket.h"
#include <stdio.h>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define FAIL_LOUDLY() { perror(__FILE__ ":" STRINGIZE(__LINE__)); raise(SIGKILL); }

typedef struct {
    unsigned int refs;

    json_t *jsondata;
    void *bindata;
    size_t bindata_len;
} dime_rcmessage_t;

typedef struct {
    int fd;
    char *name;

    dime_socket_t sock;
    dime_deque_t queue;
} dime_client_t;

static int cmp_fd(const void *a, const void *b) {
    return (*(const int *)b) - (*(const int *)a);
}

static uint64_t hash_fd(const void *a) {
    return (*(const int *)a) * 0x9E3779B97F4A7BB9;
}

static int cmp_name(const void *a, const void *b) {
    return strcmp(a, b);
}

/*
 * Note: FNV1a is currently used here to hash strings, but since this is a
 * network-enabled program, we may want to use a randomized hashing algorithm
 * like SipHash
 */
static uint64_t hash_name(const void *a) {
    uint64_t y = 0xCBF29CE484222325;

    for (const char *s = a; *s != '\0'; s++) {
        y = (y ^ *s) * 1099511628211;
    }

    return y;
}

static jmp_buf ctrlc_env;

static void ctrlc_handler(int signum) {
    longjmp(ctrlc_env, 1);
}

static int rcmessage_apply_destroy(void *val, void *p) {
    dime_rcmessage_t *msg = val;

    msg->refs--;

    if (msg->refs == 0) {
        json_decref(msg->jsondata);
        free(msg->bindata);
        free(msg);
    }

    return 1;
}

static int client_apply_appendname(const void *key, void *val, void *p) {
    json_t *str = json_string(key);

    if (str == NULL) {
        FAIL_LOUDLY();
    }

    if (json_array_append_new(p, str) < 0) {
        FAIL_LOUDLY();
    }

    return 1;
}

static int client_apply_meta_dimeb(const void *key, void *val, void *p) {
    dime_client_t *client = val;

    dime_socket_push(&client->sock, p, NULL, 0);

    return 1;
}

static int client_apply_broadcast(const void *key, void *val, void *p) {
    struct {
        dime_client_t *client;
        dime_rcmessage_t *msg;
        int err;
    } *pp = p;

    dime_client_t *client = val;

    if (client != pp->client) {
        if (dime_deque_pushr(&client->queue, pp->msg) < 0) {
            pp->err = 1;
            return 0;
        }

        pp->msg->refs++;
    }

    return 1;
}

static int client_apply_destroy(const void *key, void *val, void *p) {
    dime_client_t *client = val;

    dime_deque_apply(&client->queue, rcmessage_apply_destroy, NULL);

    dime_socket_destroy(&client->sock);
    dime_deque_destroy(&client->queue);
    free(client->name);
    free(client);

    return 1;
}

int dime_server_init(dime_server_t *srv) {
    if (dime_table_init(&srv->fd2conn, cmp_fd, hash_fd) < 0) {
        return -1;
    }

    if (dime_table_init(&srv->name2conn, cmp_name, hash_name) < 0) {
        dime_table_destroy(&srv->fd2conn);

        return -1;
    }

    union {
        struct sockaddr_in inet;
        struct sockaddr_un uds;
    } addr;
    size_t addrlen;
    int socktype, proto;

    memset(&addr, 0, sizeof(addr));

    switch (srv->protocol) {
    case DIME_UNIX:
        addr.uds.sun_family = AF_UNIX;
        strncpy(addr.uds.sun_path, srv->pathname, sizeof(addr.uds.sun_path));
        addr.uds.sun_path[sizeof(addr.uds.sun_path) - 1] = '\0';

        socktype = AF_UNIX;
        proto = 0;
        addrlen = sizeof(struct sockaddr_un);

        break;

    case DIME_TCP:
        addr.inet.sin_family = AF_INET;
        addr.inet.sin_addr.s_addr = INADDR_ANY;
        addr.inet.sin_port = htons(srv->port);

        socktype = AF_INET;
        proto = IPPROTO_TCP;
        addrlen = sizeof(struct sockaddr_in);

        break;

    default:
        dime_table_destroy(&srv->fd2conn);
        dime_table_destroy(&srv->name2conn);

        return -1;
    }

    srv->fd = socket(socktype, SOCK_STREAM, proto);
    if (srv->fd < 0) {
        dime_table_destroy(&srv->fd2conn);
        dime_table_destroy(&srv->name2conn);

        return -1;
    }

    if (bind(srv->fd, (struct sockaddr *)&addr, addrlen) < 0) {
        close(srv->fd);
        dime_table_destroy(&srv->fd2conn);
        dime_table_destroy(&srv->name2conn);

        return -1;
    }

    if (listen(srv->fd, 5) < 0) {
        close(srv->fd);
        dime_table_destroy(&srv->fd2conn);
        dime_table_destroy(&srv->name2conn);

        return -1;
    }

    srv->serialization = DIME_NO_SERIALIZATION;

    return 0;
}

void dime_server_destroy(dime_server_t *srv) {
    if (srv->protocol == DIME_UNIX) {
        unlink(srv->pathname);
    }

    shutdown(srv->fd, SHUT_RDWR);
    close(srv->fd);

    dime_table_apply(&srv->fd2conn, client_apply_destroy, NULL);

    dime_table_destroy(&srv->fd2conn);
    dime_table_destroy(&srv->name2conn);
}

int dime_server_loop(dime_server_t *srv) {
    struct pollfd *pollfds;
    size_t pollfds_len, pollfds_cap;

    pollfds_cap = 16;
    pollfds = malloc(pollfds_cap * sizeof(struct pollfd));
    if (pollfds == NULL) {
        return -1;
    }

    void (*sigint_f)(int);
    void (*sigterm_f)(int);
    void (*sigpipe_f)(int);

    sigint_f = SIG_ERR;
    sigterm_f = SIG_ERR;
    sigpipe_f = SIG_ERR;

    if (setjmp(ctrlc_env) != 0) {
        if (sigint_f != SIG_ERR) {
            signal(SIGINT, sigint_f);
        }

        if (sigterm_f != SIG_ERR) {
            signal(SIGTERM, sigint_f);
        }

        if (sigpipe_f != SIG_ERR) {
            signal(SIGPIPE, sigpipe_f);
        }

        printf("Freeing %p\n", pollfds);
        free(pollfds);

        return 0;
    }

    sigint_f = signal(SIGINT, ctrlc_handler);
    if (sigint_f == SIG_ERR) {
        free(pollfds);

        return -1;
    }

    sigterm_f = signal(SIGTERM, ctrlc_handler);
    if (sigterm_f == SIG_ERR) {
        signal(SIGINT, sigint_f);
        free(pollfds);

        return -1;
    }

    sigpipe_f = signal(SIGPIPE, SIG_IGN);
    if (sigpipe_f == SIG_ERR) {
        signal(SIGTERM, sigterm_f);
        signal(SIGINT, sigint_f);
        free(pollfds);

        return -1;
    }

    pollfds[0].fd = srv->fd;
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;

    pollfds_len = 1;

    while (1) {
        printf("Polling... ");
        fflush(stdout);
        if (poll(pollfds, pollfds_len, -1) < 0) {
            FAIL_LOUDLY();
        }
        printf("Done!\n");

        if (pollfds[0].revents & POLLIN) {
            dime_client_t *conn = malloc(sizeof(dime_client_t));
            if (conn == NULL) {
                FAIL_LOUDLY();
            }

            conn->fd = accept(srv->fd, NULL, NULL);
            if (conn->fd < 0) {
                FAIL_LOUDLY();
            }

            if (dime_socket_init(&conn->sock, conn->fd) < 0) {
                FAIL_LOUDLY();
            }

            if (dime_deque_init(&conn->queue) < 0) {
                FAIL_LOUDLY();
            }

            conn->name = NULL;

            if (dime_table_insert(&srv->fd2conn, &conn->fd, conn) < 0) {
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

            printf("Opened socket %d\n", conn->fd);
        }

        for (size_t i = 1; i < pollfds_len; i++) {
            dime_client_t *conn = dime_table_search(&srv->fd2conn, &pollfds[i].fd);

            if (pollfds[i].revents & POLLHUP) {
                printf("Closed socket %d\n", conn->fd);

                if (dime_table_remove(&srv->fd2conn, &conn->fd) == NULL) {
                    FAIL_LOUDLY();
                }

                if (conn->name != NULL) {
                    dime_table_remove(&srv->name2conn, conn->name);
                }

                dime_deque_apply(&conn->queue, rcmessage_apply_destroy, NULL);

                close(conn->fd);
                free(conn->name);
                dime_socket_destroy(&conn->sock);
                dime_deque_destroy(&conn->queue);
                free(conn);

                pollfds[i] = pollfds[pollfds_len - 1];
                pollfds_len--;
                i--;

                continue;
            }

            if (pollfds[i].revents & POLLIN) {
                printf("Got POLLIN on %d\n", conn->fd);

                ssize_t n = dime_socket_recvpartial(&conn->sock);

                if (n < 0) {
                    FAIL_LOUDLY();
                }

                while (1) {
                    json_t *jsondata;
                    void *bindata;
                    size_t bindata_len;

                    n = dime_socket_pop(&conn->sock, &jsondata, &bindata, &bindata_len);

                    if (n > 0) {
                        const char *cmd;

                        if (json_unpack(jsondata, "{ss}", "command", &cmd) < 0) {
                            FAIL_LOUDLY();
                        }

                        printf("Got message on %d: %s\n", conn->fd, cmd);

                        if (strcmp(cmd, "register") == 0) {
                            if (conn->name != NULL) {
                                FAIL_LOUDLY();
                            }

                            const char *name, *serialization;
                            int serialization_i;

                            if (json_unpack(jsondata, "{ssss}", "name", &name, "serialization", &serialization) < 0) {
                                FAIL_LOUDLY();
                            }

                            conn->name = strdup(name);
                            if (conn->name == NULL) {
                                FAIL_LOUDLY();
                            }

                            if (dime_table_insert(&srv->name2conn, conn->name, conn) < 0) {
                                FAIL_LOUDLY();
                            }

                            if (strcmp(serialization, "matlab") == 0) {
                                serialization_i = DIME_MATLAB;
                            } else if (strcmp(serialization, "pickle") == 0) {
                                serialization_i = DIME_PICKLE;
                            } else if (strcmp(serialization, "dimeb") == 0) {
                                serialization_i = DIME_DIMEB;
                            } else {
                                FAIL_LOUDLY();
                            }

                            if (srv->serialization == DIME_NO_SERIALIZATION) {
                                srv->serialization = serialization_i;
                            } else if (srv->serialization != serialization_i) {
                                if (srv->serialization != DIME_DIMEB) {
                                    json_t *meta = json_pack("{sbss}", "meta", 1, "serialization", "dimeb");
                                    if (meta == NULL) {
                                        FAIL_LOUDLY();
                                    }

                                    dime_table_apply(&srv->name2conn, client_apply_meta_dimeb, meta);

                                    json_decref(meta);
                                }

                                srv->serialization = DIME_DIMEB;
                            }

                            switch (srv->serialization) {
                            case DIME_MATLAB:
                                serialization = "matlab";
                                break;

                            case DIME_PICKLE:
                                serialization = "pickle";
                                break;

                            case DIME_DIMEB:
                                serialization = "dimeb";
                                break;
                            }

                            json_t *response = json_pack("{siss}", "status", 0, "serialization", serialization);
                            if (response == NULL) {
                                FAIL_LOUDLY();
                            }

                            dime_socket_push(&conn->sock, response, NULL, 0);

                            json_decref(response);
                            json_decref(jsondata);
                            free(bindata);
                        } else if (strcmp(cmd, "send") == 0) {
                            const char *name;

                            if (json_unpack(jsondata, "{ss}", "name", &name) < 0) {
                                FAIL_LOUDLY();
                            }

                            dime_client_t *other = dime_table_search(&srv->name2conn, name);
                            if (other == NULL) {
                                FAIL_LOUDLY();
                            }

                            dime_rcmessage_t *msg = malloc(sizeof(dime_rcmessage_t));
                            if (msg == NULL) {
                                FAIL_LOUDLY();
                            }

                            msg->refs = 1;
                            msg->jsondata = jsondata;
                            msg->bindata = bindata;
                            msg->bindata_len = bindata_len;

                            if (dime_deque_pushr(&other->queue, msg) < 0) {
                                FAIL_LOUDLY();
                            }
                        } else if (strcmp(cmd, "broadcast") == 0) {
                            dime_rcmessage_t *msg = malloc(sizeof(dime_rcmessage_t));
                            if (msg == NULL) {
                                FAIL_LOUDLY();
                            }

                            msg->refs = 0;
                            msg->jsondata = jsondata;
                            msg->bindata = bindata;
                            msg->bindata_len = bindata_len;

                            struct {
                                dime_client_t *client;
                                dime_rcmessage_t *msg;
                                int err;
                            } pp;

                            pp.client = conn;
                            pp.msg = msg;
                            pp.err = 0;

                            dime_table_apply(&srv->name2conn, client_apply_broadcast, &pp);

                            if (pp.err) {
                                FAIL_LOUDLY();
                            }

                            if (msg->refs == 0) {
                                json_decref(msg->jsondata);
                                free(msg->bindata);
                                free(msg);
                            }
                        } else if (strcmp(cmd, "sync") == 0) {
                            json_int_t n;

                            if (json_unpack(jsondata, "{sI}", "n", &n) < 0) {
                                FAIL_LOUDLY();
                            }

                            json_decref(jsondata);
                            free(bindata);

                            size_t max = (n >= 0 ? n : (size_t)-1);

                            for (size_t i = 0; i < max; i++) {
                                dime_rcmessage_t *msg = dime_deque_popl(&conn->queue);

                                if (msg == NULL) {
                                    break;
                                }

                                if (dime_socket_push(&conn->sock, msg->jsondata, msg->bindata, msg->bindata_len) < 0) {
                                    FAIL_LOUDLY();
                                }

                                msg->refs--;

                                if (msg->refs == 0) {
                                    json_decref(msg->jsondata);
                                    free(msg->bindata);
                                    free(msg);
                                }
                            }

                            json_t *response = json_object();
                            if (response == NULL) {
                                FAIL_LOUDLY();
                            }

                            if (dime_socket_push(&conn->sock, response, NULL, 0) < 0) {
                                FAIL_LOUDLY();
                            }

                            json_decref(response);
                        } else if (strcmp(cmd, "devices") == 0) {
                            json_decref(jsondata);
                            free(bindata);

                            jsondata = json_array();
                            if (jsondata == NULL) {
                                FAIL_LOUDLY();
                            }

                            dime_table_apply(&srv->name2conn, client_apply_appendname, jsondata);

                            json_t *response = json_pack("{so}", "devices", jsondata);
                            if (response == NULL) {
                                FAIL_LOUDLY();
                            }

                            if (dime_socket_push(&conn->sock, response, NULL, 0) < 0) {
                                FAIL_LOUDLY();
                            }

                            json_decref(response);
                        } else {
                            FAIL_LOUDLY();
                        }
                    } else if (n < 0) {
                        FAIL_LOUDLY();
                    } else {
                        break;
                    }
                }
            }

            if (pollfds[i].revents & POLLOUT) {
                printf("Got POLLOUT on %d\n", conn->fd);

                if (dime_socket_sendpartial(&conn->sock) < 0) {
                    FAIL_LOUDLY();
                }
            }

            if (dime_socket_sendlen(&conn->sock) > 0) {
                printf("POLLOUT is ON on socket %d\n", conn->fd);
                pollfds[i].events |= POLLOUT;
            } else {
                printf("POLLOUT is OFF on socket %d\n", conn->fd);
                pollfds[i].events &= ~POLLOUT;
            }

            pollfds[i].revents = 0;
        }
    }
}
