#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char **argv) {
    dime_server_t srv;

    srv.verbosity = 0;
    srv.pathname = "/tmp/dime.sock";
    srv.port = 5000;

#ifdef _WIN32
    srv.protocol = DIME_TCP;
#else
    srv.protocol = DIME_UNIX;
#endif

    int opt;

    while ((opt = getopt(argc, argv, "hdP:p:f:v")) >= 0) {
        switch (opt) {
        case 'h':
            printf("Usage: %s [-h] [-P unix/tcp] [-p port] [-f socketfile]\n",
                   argv[0]);
            return 0;

        case 'd':
            {
                pid_t pid = fork();

                if (pid < 0) {
                    perror("fork");
                    return -1;
                } else if (pid != 0) {
                    printf("Forked from main, PID is %u\n", (unsigned int)pid);
                    return 0;
                }
            }

            break;

        case 'P':
            if (strcasecmp(optarg, "unix") == 0) {
                srv.protocol = DIME_UNIX;
            } else if (strcasecmp(optarg, "tcp") == 0) {
                srv.protocol = DIME_TCP;
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
            srv.pathname = optarg;
            break;

        case 'c':
            srv.verbosity++;
            break;

        default:
            goto usage_err;
        }
    }

    if (optind < argc) {
        goto usage_err;
    }

    if (dime_server_init(&srv) < 0) {
        perror("dime_server_init");

        return -1;
    }

    if (dime_server_loop(&srv) < 0) {
        perror("dime_server_loop");
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
