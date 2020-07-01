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
#include <assert.h>

#include <jansson.h>

#include "client.h"
#include "server.h"
#include "table.h"
#include "deque.h"
#include "socket.h"
#include "log.h"

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

int dime_server_init(dime_server_t *srv) {
    if (dime_table_init(&srv->fd2clnt, cmp_fd, hash_fd) < 0) {
        printf("%d\n", __LINE__); return -1;
    }

    if (dime_table_init(&srv->name2clnt, cmp_name, hash_name) < 0) {
        dime_table_destroy(&srv->fd2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    union {
        struct sockaddr_in inet;
        struct sockaddr_in6 inet6;
        struct sockaddr_un uds;
    } addr;
    socklen_t addrlen;
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
        addr.inet6.sin6_family = AF_INET6;
        addr.inet6.sin6_addr = in6addr_any;
        addr.inet6.sin6_port = htons(srv->port);

        socktype = AF_INET6;
        proto = IPPROTO_TCP;
        addrlen = sizeof(struct sockaddr_in6);

        break;

    default:
        dime_table_destroy(&srv->fd2clnt);
        dime_table_destroy(&srv->name2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    srv->fd = socket(socktype, SOCK_STREAM, proto);
    if (srv->fd < 0) {
        dime_table_destroy(&srv->fd2clnt);
        dime_table_destroy(&srv->name2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    /* Fallback to IPv4 if dual-stack IPv4/IPv6 is not suppoerted */
    if (srv->protocol == DIME_TCP) {
        int no = 0;

        if (setsockopt(srv->fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(int)) < 0) {
            if (srv->verbosity >= 1) {
                dime_warn("Failed to initialize IPv4/IPv6 dual-stack, falling back to IPv4");
            }

            close(srv->fd);

            memset(&addr, 0, sizeof(addr));

            addr.inet.sin_family = AF_INET;
            addr.inet.sin_addr.s_addr = INADDR_ANY;
            addr.inet.sin_port = htons(srv->port);

            socktype = AF_INET;
            addrlen = sizeof(struct sockaddr_in);

            srv->fd = socket(socktype, SOCK_STREAM, proto);
            if (srv->fd < 0) {
                dime_table_destroy(&srv->fd2clnt);
                dime_table_destroy(&srv->name2clnt);

                printf("%d\n", __LINE__); return -1;
            }
        }
    }

    if (bind(srv->fd, (struct sockaddr *)&addr, addrlen) < 0) {
        close(srv->fd);
        dime_table_destroy(&srv->fd2clnt);
        dime_table_destroy(&srv->name2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    if (listen(srv->fd, 5) < 0) {
        close(srv->fd);
        dime_table_destroy(&srv->fd2clnt);
        dime_table_destroy(&srv->name2clnt);

        printf("%d\n", __LINE__); return -1;
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

    dime_table_iter_t it;

    dime_table_iter_init(&it, &srv->fd2clnt);

    while (dime_table_iter_next(&it)) {
        dime_client_destroy(it.val);
        free(it.val);
    }

    dime_table_iter_init(&it, &srv->name2clnt);

    while (dime_table_iter_next(&it)) {
        dime_group_t *group = it.val;

        free(group->name);
        free(group->clnts);
        free(group);
    }

    dime_table_destroy(&srv->fd2clnt);
    dime_table_destroy(&srv->name2clnt);
}

int dime_server_loop(dime_server_t *srv) {
    struct pollfd *pollfds;
    size_t pollfds_len, pollfds_cap;

    pollfds_cap = 16;
    pollfds = malloc(pollfds_cap * sizeof(struct pollfd));
    if (pollfds == NULL) {
        printf("%d\n", __LINE__); return -1;
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

        free(pollfds);
        return 0;
    }

    sigint_f = signal(SIGINT, ctrlc_handler);
    if (sigint_f == SIG_ERR) {
        free(pollfds);

        printf("%d\n", __LINE__); return -1;
    }

    sigterm_f = signal(SIGTERM, ctrlc_handler);
    if (sigterm_f == SIG_ERR) {
        signal(SIGINT, sigint_f);
        free(pollfds);

        printf("%d\n", __LINE__); return -1;
    }

    sigpipe_f = signal(SIGPIPE, SIG_IGN);
    if (sigpipe_f == SIG_ERR) {
        signal(SIGTERM, sigterm_f);
        signal(SIGINT, sigint_f);
        free(pollfds);

        printf("%d\n", __LINE__); return -1;
    }

    pollfds[0].fd = srv->fd;
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;

    pollfds_len = 1;

    while (1) {
        if (poll(pollfds, pollfds_len, -1) < 0) {
            signal(SIGPIPE, sigpipe_f);
            signal(SIGTERM, sigterm_f);
            signal(SIGINT, sigint_f);
            free(pollfds);

            printf("%d\n", __LINE__); return -1;
        }

        if (pollfds[0].revents & POLLIN) {
            dime_client_t *clnt = malloc(sizeof(dime_client_t));
            if (clnt == NULL) {
                signal(SIGPIPE, sigpipe_f);
                signal(SIGTERM, sigterm_f);
                signal(SIGINT, sigint_f);
                free(pollfds);

                printf("%d\n", __LINE__); return -1;
            }

            struct sockaddr_storage addr;
            socklen_t siz = sizeof(struct sockaddr_storage);

            int fd = accept(srv->fd, (struct sockaddr *)&addr, &siz);
            if (fd < 0) {
                free(clnt);
                signal(SIGPIPE, sigpipe_f);
                signal(SIGTERM, sigterm_f);
                signal(SIGINT, sigint_f);
                free(pollfds);

                printf("%d\n", __LINE__); return -1;
            }

            if (dime_client_init(clnt, fd, (struct sockaddr *)&addr) < 0) {
                close(fd);
                free(clnt);
                signal(SIGPIPE, sigpipe_f);
                signal(SIGTERM, sigterm_f);
                signal(SIGINT, sigint_f);
                free(pollfds);

                printf("%d\n", __LINE__); return -1;
            }

            if (dime_table_insert(&srv->fd2clnt, &clnt->fd, clnt) < 0) {
                dime_client_destroy(clnt);
                free(clnt);
                signal(SIGPIPE, sigpipe_f);
                signal(SIGTERM, sigterm_f);
                signal(SIGINT, sigint_f);
                free(pollfds);

                printf("%d\n", __LINE__); return -1;
            }

            if (pollfds_len >= pollfds_cap) {
                size_t ncap = (3 * pollfds_cap) / 2;

                void *narr = realloc(pollfds, ncap * sizeof(struct pollfd));
                if (narr == NULL) {
                    dime_client_destroy(clnt);
                    free(clnt);
                    signal(SIGPIPE, sigpipe_f);
                    signal(SIGTERM, sigterm_f);
                    signal(SIGINT, sigint_f);
                    free(pollfds);

                    printf("%d\n", __LINE__); return -1;
                }

                pollfds = narr;
                pollfds_cap = ncap;
            }

            pollfds[pollfds_len].fd = clnt->fd;
            pollfds[pollfds_len].events = POLLIN;
            pollfds[pollfds_len].revents = 0;
            pollfds_len++;

            if (srv->verbosity >= 1) {
                dime_info("Opened new connection from %s", clnt->addr);
            }
        }

        for (size_t i = 1; i < pollfds_len; i++) {
            dime_client_t *clnt = dime_table_search(&srv->fd2clnt, &pollfds[i].fd);
            assert(clnt != NULL);

            if (pollfds[i].revents & POLLHUP) {
                if (srv->verbosity >= 1) {
                    dime_info("Closed connection from %s", clnt->addr);
                }

                dime_table_remove(&srv->fd2clnt, &clnt->fd);

                dime_client_destroy(clnt);
                free(clnt);

                pollfds[i] = pollfds[pollfds_len - 1];
                pollfds_len--;
                i--;

                continue;
            }

            if (pollfds[i].revents & POLLIN) {
                ssize_t n = dime_socket_recvpartial(&clnt->sock);

                if (n < 0) {
                    signal(SIGPIPE, sigpipe_f);
                    signal(SIGTERM, sigterm_f);
                    signal(SIGINT, sigint_f);
                    free(pollfds);

                    printf("%d\n", __LINE__); return -1;
                } else if (n == 0) {
                    if (srv->verbosity >= 1) {
                        dime_info("Connection closed from %s", clnt->addr);
                    }

                    dime_table_remove(&srv->fd2clnt, &clnt->fd);

                    dime_client_destroy(clnt);
                    free(clnt);

                    pollfds[i] = pollfds[pollfds_len - 1];
                    pollfds_len--;
                    i--;

                    continue;
                }

                if (srv->verbosity >= 3) {
                    dime_info("Received %zd bytes of data from %s", n, clnt->addr);
                }

                while (1) {
                    json_t *jsondata;
                    void *bindata;
                    size_t bindata_len;

                    n = dime_socket_pop(&clnt->sock, &jsondata, &bindata, &bindata_len);

                    if (n > 0) {
                        const char *cmd;

                        if (json_unpack(jsondata, "{ss}", "command", &cmd) < 0) {
                            /* Just let this case propagate, it'll be caught below */
                            cmd = "";
                        }

                        if (srv->verbosity >= 3) {
                            dime_info("Got DiME message with command \"%s\" from %s", cmd, clnt->addr);
                        }

                        int err;

                        /*
                         * As more commands are added, this section of code
                         * might be more efficient as a table of function
                         * pointers
                         */
                        if (strcmp(cmd, "register") == 0) {
                            err = dime_client_register(clnt, srv, jsondata, &bindata, bindata_len);
                        }else if (strcmp(cmd, "join") == 0) {
                            err = dime_client_join(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "leave") == 0) {
                            err = dime_client_leave(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "send") == 0) {
                            err = dime_client_send(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "broadcast") == 0) {
                            err = dime_client_broadcast(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "sync") == 0) {
                            err = dime_client_sync(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "wait") == 0) {
                            err = dime_client_wait(clnt, srv, jsondata, &bindata, bindata_len);
                        } else if (strcmp(cmd, "devices") == 0) {
                            err = dime_client_devices(clnt, srv, jsondata, &bindata, bindata_len);
                        } else {
                            err = -1;

                            strlcpy(srv->err, "Unknown command", sizeof(srv->err));

                            json_t *response = json_pack("{siss}", "status", -1, "error", "Unknown command");
                            if (response != NULL) {
                                dime_socket_push(&clnt->sock, response, NULL, 0);
                                json_decref(response);
                            }
                        }

                        json_decref(jsondata);
                        free(bindata);

                        if (err < 0 && srv->verbosity >= 1) {
                            dime_warn("Failed to handle command \"%s\" from %s: %s", cmd, clnt->addr, srv->err);
                        }
                    } else if (n < 0) {
                        signal(SIGPIPE, sigpipe_f);
                        signal(SIGTERM, sigterm_f);
                        signal(SIGINT, sigint_f);
                        free(pollfds);

                        printf("%d\n", __LINE__); return -1;
                    } else {
                        break;
                    }
                }
            }

            if (pollfds[i].revents & POLLOUT) {
                ssize_t n = dime_socket_sendpartial(&clnt->sock);
                if (n < 0) {
                    signal(SIGPIPE, sigpipe_f);
                    signal(SIGTERM, sigterm_f);
                    signal(SIGINT, sigint_f);
                    free(pollfds);

                    printf("%d\n", __LINE__); return -1;
                }

                if (srv->verbosity >= 3) {
                    dime_info("Sent %zd bytes of data to %s", n, clnt->addr);
                }
            }
        }

        /* Iterate in reverse order for better cache locality */
        for (size_t i = pollfds_len - 1; i > 0; i--) {
            dime_client_t *clnt = dime_table_search(&srv->fd2clnt, &pollfds[i].fd);
            assert(clnt != NULL);

            if (dime_socket_sendlen(&clnt->sock) > 0) {
                pollfds[i].events |= POLLOUT;
            } else {
                pollfds[i].events &= ~POLLOUT;
            }

            pollfds[i].revents = 0;
        }
    }
}
