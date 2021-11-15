#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "Ws2_32.lib")
#endif

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/ssl.h>
#include "server.h"

static dime_server_t srv;

static void sighandler(int signal) {
    exit(0);
}

static void cleanup() {
    EVP_cleanup();
    dime_server_destroy(&srv);
}

int main(int argc, char **argv) {
    SSL_library_init();
    SSL_load_error_strings();
    atexit(cleanup);

    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

#ifdef SIGHUP
    signal(SIGHUP, sighandler);
#endif

    char *listens[(argc + 1) / 2];
    size_t listens_len = 0;
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

    memset(&srv, 0, sizeof(dime_server_t));

    srv.verbosity = 0;
    srv.threads = 1;

    for (int argi = 1; argi < argc; argi++) {
        int skip = 0;

        if (argv[argi][0] == '-') {
            for (unsigned int j = 1; argv[argi][j] != '\0'; j++) {
                switch (argv[argi][j]) {
                case 'c':
                    if (argi + 1 > argc) {
                        goto usage_err;
                    }

                    skip = 1;
                    srv.tls = 1;
                    srv.certname = argv[argi + 1];

                    break;

                case 'd':
                    srv.daemon = 1;
                    break;

                case 'h':
                    goto usage_err;

                case 'j':
                    if (argi + 1 > argc) {
                        goto usage_err;
                    }

                    skip = 1;
                    srv.threads = strtoul(argv[argi + 1], NULL, 0);
                    if (srv.threads == 0) {
                        goto usage_err;
                    }

                    break;

                case 'k':
                    if (argi + 1 > argc) {
                        goto usage_err;
                    }

                    skip = 1;
                    srv.tls = 1;
                    srv.privkeyname = argv[argi + 1];

                    break;

                case 'l':
                    if (argi + 1 > argc) {
                        goto usage_err;
                    }

                    skip = 1;
                    listens[listens_len++] = argv[argi + 1];

                    break;

                case 'v':
                    srv.verbosity++;
                    break;

                default:
                    goto usage_err;
                }
            }
        } else {
            goto usage_err;
        }

        argi += skip;
    }

    /*int opt;

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
            listens[listens_len] = optarg;
            listens_len++;
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
    }*/



    if (listens_len == 0) {
        listens[0] = listens_default;
        listens_len = 1;
    }

    if (dime_server_init(&srv) < 0) {
        fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

        return -1;
    }

    for (size_t i = 0; i < listens_len; i++) {
        char *type;

        type = strtok(listens[i], ":");

        if (strcmp(type, "unix") == 0 || strcmp(type, "ipc") == 0) {
            const char *pathname = strtok(NULL, ":");
            if (pathname == NULL) {
                goto usage_err;
            }

            if (strtok(NULL, ":") != NULL) {
                goto usage_err;
            }

            if (dime_server_add(&srv, DIME_UNIX, pathname) < 0) {
                fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

                return -1;
            }

        } else if (strcmp(type, "tcp") == 0) {
            const char *port_s = strtok(NULL, ":");
            if (port_s == NULL) {
                goto usage_err;
            }

            if (strtok(NULL, ":") != NULL) {
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
        } else if (strcmp(type, "ws") == 0) {
            const char *port_s = strtok(NULL, ":");
            if (port_s == NULL) {
                goto usage_err;
            }

            if (strtok(NULL, ":") != NULL) {
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
        return -1;
    }

    return 0;

usage_err:
    fprintf(stderr, "Usage: %s [options]\nTry \"%s -h\" for more information\n",
            argv[0], argv[0]);
    return -1;
}
