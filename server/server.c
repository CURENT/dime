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
#include <fcntl.h>
#include <stdarg.h>

#include <jansson.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

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

    srv->fds_len = 0;
    srv->fds_cap = 8;
    srv->fds = malloc(srv->fds_cap * sizeof(dime_server_fd_t));
    if (srv->fds == NULL) {
        dime_table_destroy(&srv->name2clnt);
        dime_table_destroy(&srv->fd2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    srv->pathnames_len = 0;
    srv->pathnames_cap = 8;
    srv->pathnames = malloc(srv->pathnames_cap * sizeof(char *));
    if (srv->pathnames == NULL) {
        free(srv->fds);
        dime_table_destroy(&srv->name2clnt);
        dime_table_destroy(&srv->fd2clnt);

        printf("%d\n", __LINE__); return -1;
    }

    if (srv->daemon) {
        pid_t pid = fork();

        if (pid < 0) {
            close(srv->fd);
            dime_table_destroy(&srv->fd2clnt);
            dime_table_destroy(&srv->name2clnt);

            printf("%d\n", __LINE__); return -1;
        } else if (pid != 0) {
            if (srv->verbosity >= 1) {
                dime_info("Forked from main, PID is %d", (int)pid);
            }

            exit(0);
        }
    }

/*    if (srv->tls) {
        if (srv->certname == NULL) {
            if (srv->verbosity >= 1) {
                dime_warn("Certificate file not given, TLS will be disabled");
            }

            goto tls_break;
        }

        if (srv->privkeyname == NULL) {
            if (srv->verbosity >= 1) {
                dime_warn("Private key file not given, TLS will be disabled");
            }

            goto tls_break;
        }


        srv->tlsctx = SSL_CTX_new(TLS_server_method());

        if (srv->tlsctx == NULL) {
            if (srv->verbosity >= 1) {
                dime_warn("Failed to initalize TLS: %s", ERR_reason_error_string(ERR_get_error()));
            }

            goto tls_break;
        }

        SSL_CTX_set_ecdh_auto(srv->tlsctx, 1);

        if (SSL_CTX_use_certificate_file(srv->tlsctx, srv->certname, SSL_FILETYPE_PEM) <= 0) {
            if (srv->verbosity >= 1) {
                dime_warn("Failed to initalize TLS: %s", ERR_reason_error_string(ERR_get_error()));
            }

            SSL_CTX_free(srv->tlsctx);
            srv->tlsctx = NULL;

            goto tls_break;
        }

        if (SSL_CTX_use_PrivateKey_file(srv->tlsctx, srv->privkeyname, SSL_FILETYPE_PEM) <= 0) {
            if (srv->verbosity >= 1) {
                dime_warn("Failed to initalize TLS: %s", ERR_reason_error_string(ERR_get_error()));
            }

            SSL_CTX_free(srv->tlsctx);
            srv->tlsctx = NULL;

            goto tls_break;
        }
    }
tls_break:*/

    srv->serialization = DIME_NO_SERIALIZATION;

    return 0;
}

void dime_server_destroy(dime_server_t *srv) {
    for (size_t i = 0; i < srv->pathnames_len; i++) {
        unlink(srv->pathnames[i]);
        free(srv->pathnames[i]);
    }

    free(srv->pathnames);

    for (size_t i = 0; i < srv->fds_len; i++) {
        close(srv->fds[i].fd);
    }

    free(srv->fds);

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

int dime_server_add(dime_server_t *srv, int protocol, ...) {
    if (srv->fds_len >= srv->fds_cap) {
        size_t ncap = (srv->fds_cap * 3) / 2;
        dime_server_fd_t *narr = realloc(srv->fds, ncap * sizeof(dime_server_fd_t));
        if (narr == NULL) {
            printf("%d\n", __LINE__); return -1;
        }

        srv->fds = narr;
        srv->fds_cap = ncap;
    }

    va_list args;
    va_start(args, protocol);

    int fd;

    switch (protocol) {
    case DIME_UNIX:
        {
            const char *pathname = va_arg(args, const char *);
            va_end(args);

            struct sockaddr_un addr;

            memset(&addr, 0, sizeof(struct sockaddr_un));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, pathname, sizeof(addr.sun_path));
            addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

            char *pathname_copy = strdup(addr.sun_path);
            if (pathname_copy == NULL) {
                printf("%d\n", __LINE__); return -1;
            }

            if (srv->pathnames_len >= srv->pathnames_cap) {
                size_t ncap = (srv->pathnames_cap * 3) / 2;
                char **narr = realloc(srv->pathnames, ncap * sizeof(char *));
                if (narr == NULL) {
                    free(pathname_copy);

                    printf("%d\n", __LINE__); return -1;
                }

                srv->pathnames = narr;
                srv->pathnames_cap = ncap;
            }

            fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd < 0) {
                free(pathname_copy);

                printf("%d\n", __LINE__); return -1;
            }

            if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
                close(fd);
                free(pathname_copy);

                printf("%d\n", __LINE__); return -1;
            }

            srv->pathnames[srv->pathnames_len++] = pathname_copy;
        }
        break;

    case DIME_TCP:
    case DIME_WS:
        {
            uint16_t port = va_arg(args, unsigned int);
            va_end(args);

            union {
                struct sockaddr_in ipv4;
                struct sockaddr_in6 ipv6;
            } addr;
            socklen_t addrlen;

            memset(&addr, 0, sizeof(struct sockaddr_in6));

            addr.ipv6.sin6_family = AF_INET6;
            addr.ipv6.sin6_addr = in6addr_any;
            addr.ipv6.sin6_port = htons(port);
            addrlen = sizeof(struct sockaddr_in6);

            fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (fd < 0) {
                printf("%d\n", __LINE__); return -1;
            }

            int no = 0;

            if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(int)) < 0) {
                if (srv->verbosity >= 1) {
                    dime_warn("Failed to initialize IPv4/IPv6 dual-stack, falling back to IPv4");
                }

                close(fd);

                memset(&addr, 0, sizeof(struct sockaddr_in));

                addr.ipv4.sin_family = AF_INET;
                addr.ipv4.sin_addr.s_addr = INADDR_ANY;
                addr.ipv4.sin_port = htons(port);
                addrlen = sizeof(struct sockaddr_in);

                fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (fd < 0) {
                    printf("%d\n", __LINE__); return -1;
                }
            }

            if (bind(fd, (struct sockaddr *)&addr, addrlen) < 0) {
                close(fd);

                printf("%d\n", __LINE__); return -1;
            }
        }
        break;

    default:
        va_end(args);
        printf("%d\n", __LINE__); return -1;
    }

    srv->fds[srv->fds_len].fd = fd;
    srv->fds[srv->fds_len].protocol = protocol;
    srv->fds_len++;

    return 0;
}

int dime_server_loop(dime_server_t *srv) {
    if (srv->fds_len == 0) {
        return 0;
    }

    struct pollfd *pollfds;
    size_t pollfds_len, pollfds_cap;

    pollfds_cap = srv->fds_len + 8;
    pollfds = malloc(pollfds_cap * sizeof(struct pollfd));
    if (pollfds == NULL) {
        printf("%d\n", __LINE__); return -1;
    }

    void (*sigint_f)(int);
    void (*sigterm_f)(int);
    void (*sigpipe_f)(int);

    sigint_f = NULL;
    sigterm_f = NULL;
    sigpipe_f = NULL;

    if (setjmp(ctrlc_env) != 0) {
        if (sigint_f != NULL) {
            signal(SIGINT, sigint_f);
        }

        if (sigterm_f != NULL) {
            signal(SIGTERM, sigint_f);
        }

        if (sigpipe_f != NULL) {
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

    for (size_t i = 0; i < srv->fds_len; i++) {
        pollfds[i].fd = srv->fds[i].fd;
        pollfds[i].events = POLLIN;
        pollfds[i].revents = 0;

        if (listen(pollfds[i].fd, 0) < 0) {
            signal(SIGPIPE, sigpipe_f);
            signal(SIGTERM, sigterm_f);
            signal(SIGINT, sigint_f);
            free(pollfds);

            printf("%d\n", __LINE__); return -1;
        }
    }

    pollfds_len = srv->fds_len;

    while (1) {
        if (poll(pollfds, pollfds_len, -1) < 0) {
            signal(SIGPIPE, sigpipe_f);
            signal(SIGTERM, sigterm_f);
            signal(SIGINT, sigint_f);
            free(pollfds);

            printf("%d\n", __LINE__); return -1;
        }

        for (size_t i = 0; i < srv->fds_len; i++) {
            if (pollfds[i].revents & POLLIN) {
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

                int fd = accept(srv->fds[i].fd, (struct sockaddr *)&addr, &siz);
                if (fd < 0) {
                    free(clnt);
                    signal(SIGPIPE, sigpipe_f);
                    signal(SIGTERM, sigterm_f);
                    signal(SIGINT, sigint_f);
                    free(pollfds);

                    printf("%d\n", __LINE__); return -1;
                }

                /* Attempt to make sockets non-blocking for network connections */
                if (srv->fds[i].protocol != DIME_UNIX) {
                    int flags = fcntl(fd, F_GETFL, 0);

                    if (flags >= 0) {
                        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                    }
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

                if (srv->fds[i].protocol == DIME_WS) {
                    if (dime_socket_init_ws(&clnt->sock) < 0) {
                        dime_client_destroy(clnt);
                        free(clnt);
                        signal(SIGPIPE, sigpipe_f);
                        signal(SIGTERM, sigterm_f);
                        signal(SIGINT, sigint_f);
                        free(pollfds);

                        printf("%d\n", __LINE__); return -1;
                    }
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
                    struct pollfd *narr = realloc(pollfds, ncap * sizeof(struct pollfd));
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
        }

        for (size_t i = srv->fds_len; i < pollfds_len; i++) {
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

                if (n <= 0) {
                    if (srv->verbosity >= 1) {
                        if (n == 0) {
                            dime_info("Connection closed from %s", clnt->addr);
                        } else {
                            dime_err("Read failed on %s (%s), closing", clnt->addr, strerror(errno));
                        }
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
                        if (strcmp(cmd, "handshake") == 0) {
                            err = dime_client_handshake(clnt, srv, jsondata, &bindata, bindata_len);
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

                            strncpy(srv->err, "Unknown command", sizeof(srv->err));
                            srv->err[sizeof(srv->err) - 1] = '\0';

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

                /* Note: The server should close the socket here, not crash */
                if (n < 0) {
                    if (srv->verbosity >= 1) {
                        dime_err("Write failed on %s (%s), closing", clnt->addr, strerror(errno));
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
                    dime_info("Sent %zd bytes of data to %s", n, clnt->addr);
                }
            }
        }

        /* Iterate in reverse order for better cache locality */
        for (size_t i = pollfds_len - 1; i >= srv->fds_len; i--) {
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
