#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <openssl/ssl.h>
#include "server.h"

static void cleanup() {
    EVP_cleanup();
}

int main(int argc, char **argv) {
    SSL_library_init();
    SSL_load_error_strings();
    atexit(cleanup);

    dime_server_t srv;

    memset(&srv, 0, sizeof(dime_server_t));

    srv.socketname = "/tmp/dime.sock";
    srv.port = 5000;
    srv.verbosity = 0;
    srv.threads = 1;

#ifdef _WIN32
    srv.protocol = DIME_TCP;
#else
    srv.protocol = DIME_UNIX;
#endif

    int opt;

    while ((opt = getopt(argc, argv, "P:c:df:hj:k:p:v")) >= 0) {
        switch (opt) {
        case 'h':
            printf("Usage: %s [-h] [-P unix/tcp] [-p port] [-f socketfile]\n",
                   argv[0]);
            return 0;

        case 'd':
            srv.daemon = 1;
            break;

        case 'P':
            if (strcasecmp(optarg, "unix") == 0) {
                srv.protocol = DIME_UNIX;
            } else if (strcasecmp(optarg, "tcp") == 0) {
                srv.protocol = DIME_TCP;
            } else if (strcasecmp(optarg, "ws") == 0) {
                srv.protocol = DIME_WS;
            } else {
                goto usage_err;
            }

            break;

        case 'p':
            srv.port = strtoul(optarg, NULL, 0);
            if (srv.port == 0) {
                goto usage_err;
            }

            break;

        case 'f':
            srv.socketname = optarg;
            break;

        case 'v':
            srv.verbosity++;
            break;

        case 'c':
            srv.tls = 1;
            srv.certname = optarg;
            break;

        case 'k':
            srv.tls = 1;
            srv.privkeyname = optarg;
            break;

        default:
            goto usage_err;
        }
    }

    if (optind < argc) {
        goto usage_err;
    }

    if (dime_server_init(&srv) < 0) {
        fprintf(stderr, "Fatal error while initializing server: %s\n", srv.err);

        return -1;
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
