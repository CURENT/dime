#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "server.h"

int main(int argc, char **argv) {
    int protocol;
    uint16_t port;
    const char *socketfile;

    signal(SIGPIPE, SIG_IGN);

#ifdef _WIN32
    protocol = DIME_TCP;
#else
    protocol = DIME_UNIX;
#endif
    socketfile = "/tmp/dime.sock";
    port = 5000;

    int opt;

    while ((opt = getopt(argc, argv, "hdP:p:f:")) >= 0) {
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
                protocol = DIME_UNIX;
            } else if (strcasecmp(optarg, "tcp") == 0) {
                protocol = DIME_TCP;
            } else {
                goto usage_err;
            }

            break;

        case 'p':
            port = strtoul(optarg, NULL, 0);
            if (port == 0) {
                goto usage_err;
            }

            break;

        case 'f':
            socketfile = optarg;
            break;

        default:
            goto usage_err;
        }
    }

    if (optind < argc) {
        goto usage_err;
    }

    dime_server_t *srv = dime_server_new(protocol, socketfile, port);
    if (srv == NULL) {
        perror("Failed to setup server");
        return -1;
    }

    dime_server_loop(srv);
    dime_server_free(srv);
    return 0;

usage_err:
    fprintf(stderr, "Usage: %s [-h] [-P unix/tcp] [-p port] [-f socketfile]\n",
            argv[0]);
    return -1;
}
