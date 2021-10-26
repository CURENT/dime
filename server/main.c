#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#endif

#   include <unistd.h>
#   include <strings.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/ssl.h>
#include "server.h"

static void cleanup() {
    EVP_cleanup();
}

int main(int argc, char **argv) {
    SSL_library_init();
    SSL_load_error_strings();
    atexit(cleanup);

    char *listens[(argc + 1) / 2];
    size_t listens_i = 0;
#ifdef _WIN32
    char listens_default[] = "tcp:5000";
    WSADATA _d;

    if (WSAStartup(MAKEWORD(2, 2), &_d)) {
        fputs("Fatal error while initializing server: Failed to initialize WinSock2", stderr);
        return -1;
    }
#else
    char listens_default[] = "unix:/tmp/dime.sock";
#endif

    dime_server_t srv;

    memset(&srv, 0, sizeof(dime_server_t));

    srv.verbosity = 0;
    srv.threads = 1;

    int opt;

    while ((opt = getopt(argc, argv, "c:dhj:k:l:v")) >= 0) {
        switch (opt) {
        case 'c':
            srv.tls = 1;
            srv.certname = optarg;
            break;

        case 'd':
            srv.daemon = 1;
            break;

        case 'h':
            printf("Usage: %s [options]\n"
                   "Options:\n"
                   "",
                   argv[0]);
            return 0;

        case 'j':
            srv.threads = strtoul(optarg, NULL, 0);
            if (srv.threads == 0) {
                goto usage_err;
            }

            break;

        case 'k':
            srv.tls = 1;
            srv.privkeyname = optarg;
            break;

        case 'l':
            listens[listens_i] = optarg;
            listens_i++;
            break;

        case 'v':
            srv.verbosity++;
            break;

        default:
            goto usage_err;
        }
    }

    if (optind < argc) {
        goto usage_err;
    }

    if (listens_i == 0) {
        listens[0] = listens_default;
        listens_i = 1;
    }

    if (dime_server_init(&srv) < 0) {
        fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

        return -1;
    }

    for (size_t i = 0; i < listens_i; i++) {
        char *type, *saveptr;

        type = strtok_r(listens[i], ":", &saveptr);

        if (strcasecmp(type, "unix") == 0 || strcasecmp(type, "ipc") == 0) {
            const char *pathname = strtok_r(NULL, ":", &saveptr);
            if (pathname == NULL) {
                goto usage_err;
            }

            if (strtok_r(NULL, ":", &saveptr) != NULL) {
                goto usage_err;
            }

            if (dime_server_add(&srv, DIME_UNIX, pathname) < 0) {
                fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

                return -1;
            }

        } else if (strcasecmp(type, "tcp") == 0) {
            const char *port_s = strtok_r(NULL, ":", &saveptr);
            if (port_s == NULL) {
                goto usage_err;
            }

            if (strtok_r(NULL, ":", &saveptr) != NULL) {
                goto usage_err;
            }

            uint16_t port = strtoul(port_s, NULL, 0);
            if (port == 0) {
                goto usage_err;
            }

            if (dime_server_add(&srv, DIME_TCP, port) < 0) {
                fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

                return -1;
            }
        } else if (strcasecmp(type, "ws") == 0) {
            const char *port_s = strtok_r(NULL, ":", &saveptr);
            if (port_s == NULL) {
                goto usage_err;
            }

            if (strtok_r(NULL, ":", &saveptr) != NULL) {
                goto usage_err;
            }

            uint16_t port = strtoul(port_s, NULL, 0);
            if (port == 0) {
                goto usage_err;
            }

            if (dime_server_add(&srv, DIME_WS, port) < 0) {
                fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

                return -1;
            }
        } else {
            goto usage_err;
        }
    }

    if (dime_server_loop(&srv) < 0) {
        fprintf(stderr, "Fatal error while running server: %s\n", srv.err);
        dime_server_destroy(&srv);

        return -1;
    }

    dime_server_destroy(&srv);
    return 0;

usage_err:
    fprintf(stderr, "Usage: %s [options]\nTry \"%s -h\" for more information\n",
            argv[0], argv[0]);
    return -1;
}
