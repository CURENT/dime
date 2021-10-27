/*
#ifdef __linux__
#   define _GNU_SOURCE
#endif
*/
#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "Ws2_32.lib")
#else
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <unistd.h>
#   include <sys/socket.h>
#   include <sys/un.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include "client.h"
#include "deque.h"
#include "log.h"
#include "server.h"
#include "socket.h"
#include "table.h"

int dime_client_init(dime_client_t *clnt, int fd, const struct sockaddr *addr) {
    clnt->fd = fd;
    clnt->waiting = 0;
    clnt->err[0] = '\0';

    switch (addr->sa_family) {
    case AF_INET6:
        {
            clnt->addr = malloc(40);
            if (clnt->addr == NULL) {
                return -1;
            }

            struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)addr;
            char tmp[34];

            inet_ntop(AF_INET6, &inet6->sin6_addr, tmp, sizeof(tmp));
            snprintf(clnt->addr, 40, "%s:%hu", tmp, (unsigned short)(inet6->sin6_port));
        }

        break;

    case AF_INET:
        {
            clnt->addr = malloc(22);
            if (clnt->addr == NULL) {
                return -1;
            }

            struct sockaddr_in *inet = (struct sockaddr_in *)addr;
            char s[16];

            inet_ntop(AF_INET, &inet->sin_addr, s, sizeof(s));
            snprintf(clnt->addr, 22, "%s:%hu", s, (unsigned short)(inet->sin_port));
        }

        break;

#ifndef _WIN32
    case AF_UNIX:
#endif
        /*
        // Attempt to get PID of other process on Linux
#ifdef __linux__
        {
            struct ucred cred;
            socklen_t credlen = sizeof(struct ucred);

            if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &credlen) >= 0) {
                clnt->addr = malloc(30);
                if (clnt->addr == NULL) {
                    return -1;
                }

                snprintf(clnt->addr, 30, "PID %d, fd %d", (int)cred.pid, fd);

                break;
            }
        }
#endif
        */

    default:
        clnt->addr = malloc(14);
        if (clnt->addr == NULL) {
            return -1;
        }

        snprintf(clnt->addr, 14, "fd %d", fd);

        break;
    }

    clnt->groups_len = 0;
    clnt->groups_cap = 4;

    clnt->groups = malloc(sizeof(dime_group_t *) * clnt->groups_cap);
    if (clnt->groups == NULL) {
        free(clnt->addr);

        return -1;
    }

    if (dime_socket_init(&clnt->sock, fd) < 0) {
        free(clnt->groups);
        free(clnt->addr);

        return -1;
    }

    if (dime_deque_init(&clnt->queue) < 0) {
        dime_socket_destroy(&clnt->sock);
        free(clnt->groups);
        free(clnt->addr);

        return -1;
    }

    return 0;
}

void dime_client_destroy(dime_client_t *clnt) {
    for (size_t i = 0; i < clnt->groups_len; i++) {
        dime_group_t *group = clnt->groups[i];

        for (size_t j = 0; j < group->clnts_len; j++) {
            if (group->clnts[j] == clnt) {
                group->clnts_len--;
                group->clnts[j] = group->clnts[group->clnts_len];

                break;
            }
        }
    }

    dime_deque_iter_t it;

    dime_deque_iter_init(&it, &clnt->queue);

    while (dime_deque_iter_next(&it)) {
        dime_rcmessage_t *msg = it.val;

        msg->refs--;

        if (msg->refs == 0) {
            free(msg->jsondata);
            free(msg->bindata);
            free(msg);
        }
    }

    free(clnt->addr);
    free(clnt->groups);
    dime_deque_destroy(&clnt->queue);
    dime_socket_destroy(&clnt->sock);
}

int dime_client_handshake(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    const char *serialization;
    int tls;

    json_error_t err;

    if (json_unpack_ex(jsondata, &err, 0, "{sssb}", "serialization", &serialization, "tls", &tls) < 0) {
        strncpy(srv->err, "JSON parsing error: ", sizeof(srv->err));
        strncat(srv->err, err.text, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "JSON parsing error: ", err.text);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    int serialization_i;

    if (strcmp(serialization, "matlab") == 0) {
        serialization_i = DIME_MATLAB;
    } else if (strcmp(serialization, "pickle") == 0) {
        serialization_i = DIME_PICKLE;
    } else if (strcmp(serialization, "dimeb") == 0) {
        serialization_i = DIME_DIMEB;
    } else if (strcmp(serialization, "json") == 0) {
        serialization_i = DIME_JSON;
    } else {
        return -1;
    }

    if (srv->serialization == DIME_NO_SERIALIZATION) {
        srv->serialization = serialization_i;
    } else if (srv->serialization != serialization_i && srv->serialization != DIME_DIMEB && srv->serialization != DIME_JSON) {
        json_t *meta = json_pack("{sisbssss}", "status", 1, "meta", 1, "command", "reregister", "serialization", "dimeb");
        if (meta == NULL) {
            return -1;
        }

        char *meta_str = json_dumps(meta, JSON_COMPACT);
        if (meta_str == NULL) {
            json_decref(meta);

            return -1;
        }

        json_decref(meta);

        dime_table_iter_t it;
        dime_table_iter_init(&it, &srv->fd2clnt);

        while (dime_table_iter_next(&it)) {
            dime_client_t *other = it.val;

            if (other != clnt && dime_socket_push_str(&other->sock, meta_str, NULL, 0) < 0) {
                free(meta_str);

                return -1;
            }
        }

        free(meta_str);
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

    case DIME_JSON:
        serialization = "json";
        break;
    }

    tls = (tls && srv->tlsctx != NULL);

    json_t *response = json_pack("{sisssb}", "status", 0, "serialization", serialization, "tls", tls);
    if (response == NULL) {
        return -1;
    }

    if (dime_socket_push(&clnt->sock, response, NULL, 0) < 0) {
        json_decref(response);

        return -1;
    }

    json_decref(response);

    if (tls) {
        if (srv->verbosity >= 1) {
            dime_warn("Temporarily pausing event loop to handle a TLS handshake");
        }

        if (dime_socket_init_tls(&clnt->sock, srv->tlsctx) < 0) {
            return -1;
        }

        if (srv->verbosity >= 1) {
            dime_warn("TLS handshake established with %s", clnt->addr);
        }
    }

    return 0;
}

int dime_client_join(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    json_t *arr;
    json_error_t err;

    if (json_unpack_ex(jsondata, &err, 0, "{so}", "name", &arr) < 0) {
        strncpy(srv->err, "JSON parsing error: ", sizeof(srv->err));
        strncat(srv->err, err.text, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "JSON parsing error: ", err.text);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    size_t i;
    json_t *v;

    json_array_foreach(arr, i, v) {
        const char *name = json_string_value(v);
        if (name == NULL) {
            strncpy(srv->err, "JSON parsing error: expected string", sizeof(srv->err));
            srv->err[sizeof(srv->err) - 1] = '\0';

            json_t *response = json_pack("{siss}", "status", -1, "error", "JSON parsing error: expected string");
            if (response != NULL) {
                dime_socket_push(&clnt->sock, response, NULL, 0);
                json_decref(response);
            }

            return -1;
        }

        for (size_t j = 0; j < clnt->groups_len; j++) {
            if (strcmp(name, clnt->groups[j]->name) == 0) {
                strncpy(srv->err, "Client is already in group: ", sizeof(srv->err));
                strncat(srv->err, name, sizeof(srv->err) - strlen(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss+}", "status", -1, "error", "Client is already in group: ", name);
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }
        }

        dime_group_t *group = dime_table_search(&srv->name2clnt, name);
        if (group == NULL) {
            group = malloc(sizeof(dime_group_t));
            if (group == NULL) {
                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            group->name = strdup(name);
            if (group->name == NULL) {
                free(group);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            group->clnts_len = 0;
            group->clnts_cap = 4;

            group->clnts = malloc(sizeof(dime_client_t *) * group->clnts_cap);
            if (group->clnts == NULL) {
                free(group->name);
                free(group);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            if (dime_table_insert(&srv->name2clnt, group->name, group) < 0) {
                free(group->clnts);
                free(group->name);
                free(group);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }
        }

        if (clnt->groups_len >= clnt->groups_cap) {
            size_t ncap = (clnt->groups_cap * 3) / 2;

            dime_group_t **ngroups = realloc(clnt->groups, sizeof(dime_group_t *) * ncap);
            if (ngroups == NULL) {
                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            clnt->groups = ngroups;
            clnt->groups_cap = ncap;
        }

        if (group->clnts_len >= group->clnts_cap) {
            size_t ncap = (group->clnts_cap * 3) / 2;

            dime_client_t **nclnts = realloc(group->clnts, sizeof(dime_client_t *) * ncap);
            if (nclnts == NULL) {
                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            group->clnts = nclnts;
            group->clnts_cap = ncap;
        }

        clnt->groups[clnt->groups_len] = group;
        clnt->groups_len++;

        group->clnts[group->clnts_len] = clnt;
        group->clnts_len++;

        if (srv->verbosity >= 2) {
            dime_info("%s joined group \"%s\"", clnt->addr, group->name);
        }
    }

    if (dime_socket_push_str(&clnt->sock, "{\"status\":0}", NULL, 0) < 0) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    return 0;
}

int dime_client_leave(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    json_t *arr;
    json_error_t err;

    if (json_unpack_ex(jsondata, &err, 0, "{so}", "name", &arr) < 0) {
        strncpy(srv->err, "JSON parsing error: ", sizeof(srv->err));
        strncat(srv->err, err.text, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "JSON parsing error: ", err.text);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    size_t i;
    json_t *v;

    json_array_foreach(arr, i, v) {
        const char *name = json_string_value(v);
        if (name == NULL) {
            strncpy(srv->err, "JSON parsing error: expected string", sizeof(srv->err));
            srv->err[sizeof(srv->err) - 1] = '\0';

            json_t *response = json_pack("{siss}", "status", -1, "error", "JSON parsing error: expected string");
            if (response != NULL) {
                dime_socket_push(&clnt->sock, response, NULL, 0);
                json_decref(response);
            }

            return -1;
        }

        for (size_t i = 0; i < clnt->groups_len; i++) {
            dime_group_t *group = clnt->groups[i];

            if (strcmp(name, group->name) == 0) {
                for (size_t j = 0; j < group->clnts_len; j++) {
                    if (group->clnts[j] == clnt) {
                        group->clnts_len--;
                        group->clnts[j] = group->clnts[group->clnts_len];

                        clnt->groups_len--;
                        clnt->groups[i] = clnt->groups[clnt->groups_len];

                        if (srv->verbosity >= 2) {
                            dime_info("%s left group \"%s\"", clnt->addr, group->name);
                        }

                        goto next;
                    }
                }

                assert(0); /* This should be unreachable */
            }
        }

        strncpy(srv->err, "Client is not in group: ", sizeof(srv->err));
        strncat(srv->err, name, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "Client is not in group: ", name);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;

    next:
        ;
    }

    if (dime_socket_push_str(&clnt->sock, "{\"status\":0}", NULL, 0) < 0) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    return 0;
}

int dime_client_send(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    const char *name;
    json_error_t err;

    if (json_unpack_ex(jsondata, &err, 0, "{ss}", "name", &name) < 0) {
        strncpy(srv->err, "JSON parsing error: ", sizeof(srv->err));
        strncat(srv->err, err.text, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "JSON parsing error: ", err.text);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    dime_group_t *group = dime_table_search(&srv->name2clnt, name);
    if (group == NULL || group->clnts_len == 0) {
        strncpy(srv->err, "No such group exists: ", sizeof(srv->err));
        strncat(srv->err, name, sizeof(srv->err) - strlen(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss+}", "status", -1, "error", "No such group exists: ", name);
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    dime_rcmessage_t *msg = malloc(sizeof(dime_rcmessage_t));
    if (msg == NULL) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    msg->jsondata = json_dumps(jsondata, JSON_COMPACT);
    if (msg->jsondata == NULL) {
        free(msg);

        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    msg->refs = 0;
    msg->bindata = *pbindata;
    msg->bindata_len = bindata_len;

    *pbindata = NULL;

    for (size_t i = 0; i < group->clnts_len; i++) {
        if (dime_deque_pushr(&group->clnts[i]->queue, msg) < 0) {
            if (msg->refs == 0) {
                free(msg->jsondata);
                free(msg->bindata);
                free(msg);
            }

            strncpy(srv->err, strerror(errno), sizeof(srv->err));
            srv->err[sizeof(srv->err) - 1] = '\0';

            json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
            if (response != NULL) {
                dime_socket_push(&clnt->sock, response, NULL, 0);
                json_decref(response);
            }

            return -1;
        }

        if (group->clnts[i]->waiting) {
            json_t *response = json_pack("{sisI}", "status", 0, "n", (json_int_t)dime_deque_len(&group->clnts[i]->queue));
            if (response == NULL) {
                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&group->clnts[i]->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            if (dime_socket_push(&group->clnts[i]->sock, response, NULL, 0) < 0) {
                json_decref(response);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&group->clnts[i]->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            group->clnts[i]->waiting = 0;
            json_decref(response);
        }

        msg->refs++;
    }

    assert(msg->refs > 0);

    if (srv->verbosity >= 2) {
        const char *varname;

        if (json_unpack(jsondata, "{ss}", "varname", &varname) < 0) {
            varname = "(unknown)";
        }

        dime_info("%s sent a variable \"%s\" to group \"%s\"", clnt->addr, varname, group->name);
    }

    if (dime_socket_push_str(&clnt->sock, "{\"status\":0}", NULL, 0) < 0) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    return 0;
}

int dime_client_broadcast(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    dime_rcmessage_t *msg = malloc(sizeof(dime_rcmessage_t));
    if (msg == NULL) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    msg->jsondata = json_dumps(jsondata, JSON_COMPACT);
    if (msg->jsondata == NULL) {
        free(msg);

        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    msg->refs = 0;
    msg->bindata = *pbindata;
    msg->bindata_len = bindata_len;

    *pbindata = NULL;

    dime_table_iter_t it;

    dime_table_iter_init(&it, &srv->fd2clnt);

    while (dime_table_iter_next(&it)) {
        dime_client_t *other = it.val;

        if (clnt->fd != other->fd) {
            if (dime_deque_pushr(&other->queue, msg) < 0) {
                if (msg->refs == 0) {
                    free(msg->jsondata);
                    free(msg->bindata);
                    free(msg);
                }

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            if (other->waiting) {
                json_t *response = json_pack("{sisI}", "status", 0, "n", (json_int_t)dime_deque_len(&other->queue));
                if (response == NULL) {
                    strncpy(srv->err, strerror(errno), sizeof(srv->err));
                    srv->err[sizeof(srv->err) - 1] = '\0';

                    response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                    if (response != NULL) {
                        dime_socket_push(&other->sock, response, NULL, 0);
                        json_decref(response);
                    }

                    return -1;
                }

                if (dime_socket_push(&other->sock, response, NULL, 0) < 0) {
                    json_decref(response);

                    strncpy(srv->err, strerror(errno), sizeof(srv->err));
                    srv->err[sizeof(srv->err) - 1] = '\0';

                    response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                    if (response != NULL) {
                        dime_socket_push(&other->sock, response, NULL, 0);
                        json_decref(response);
                    }

                    return -1;
                }

                other->waiting = 0;
                json_decref(response);
            }

            msg->refs++;
        }
    }

    if (msg->refs == 0) {
        free(msg->jsondata);
        free(msg->bindata);
        free(msg);
    }

    if (srv->verbosity >= 2) {
        const char *varname;

        if (json_unpack(jsondata, "{ss}", "varname", &varname) < 0) {
            varname = "(unknown)";
        }

        dime_info("%s broadcasted a variable \"%s\"", clnt->addr, varname);
    }

    if (dime_socket_push_str(&clnt->sock, "{\"status\":0}", NULL, 0) < 0) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    return 0;
}

int dime_client_sync(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    json_int_t n;

    if (json_unpack(jsondata, "{sI}", "n", &n) < 0) {
        return -1;
    }

    size_t m = (size_t)(n < 0 ? -1 : n);

    for (size_t i = 0; i < m; i++) {
        dime_rcmessage_t *msg = dime_deque_popl(&clnt->queue);

        if (msg == NULL) {
            break;
        }

        if (dime_socket_push_str(&clnt->sock, msg->jsondata, msg->bindata, msg->bindata_len) < 0) {
            dime_deque_pushl(&clnt->queue, msg);

            return -1;
        }

        msg->refs--;

        if (msg->refs == 0) {
            free(msg->jsondata);
            free(msg->bindata);
            free(msg);
        }
    }

    if (srv->verbosity >= 2) {
        if (n < 0) {
            dime_info("%s synchronized all variables", clnt->addr);
        } else {
            dime_info("%s synchronized up to %lld variables", clnt->addr, (long long)n);
        }
    }

    if (dime_socket_push_str(&clnt->sock, "{\"status\":0}", NULL, 0) < 0) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    return 0;
}

int dime_client_wait(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    if (dime_deque_len(&clnt->queue) > 0) {
        json_t *response = json_pack("{sisI}", "status", 0, "n", (json_int_t)dime_deque_len(&clnt->queue));
        if (response == NULL) {
            strncpy(srv->err, strerror(errno), sizeof(srv->err));
            srv->err[sizeof(srv->err) - 1] = '\0';

            response = json_pack("{siss}", "status", -1, "error", strerror(errno));
            if (response != NULL) {
                dime_socket_push(&clnt->sock, response, NULL, 0);
                json_decref(response);
            }

            return -1;
        }

        if (dime_socket_push(&clnt->sock, response, NULL, 0) < 0) {
            json_decref(response);

            strncpy(srv->err, strerror(errno), sizeof(srv->err));
            srv->err[sizeof(srv->err) - 1] = '\0';

            response = json_pack("{siss}", "status", -1, "error", strerror(errno));
            if (response != NULL) {
                dime_socket_push(&clnt->sock, response, NULL, 0);
                json_decref(response);
            }

            return -1;
        }

        json_decref(response);
    } else {
        clnt->waiting = 1;
    }

    return 0;
}

int dime_client_devices(dime_client_t *clnt, dime_server_t *srv, json_t *jsondata, void **pbindata, size_t bindata_len) {
    json_t *arr = json_array();
    if (arr == NULL) {
        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    dime_table_iter_t it;

    dime_table_iter_init(&it, &srv->name2clnt);

    while (dime_table_iter_next(&it)) {
        dime_group_t *group = it.val;

        if (group->clnts_len > 0) {
            json_t *str = json_string(group->name);
            if (str == NULL) {
                json_decref(arr);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }

            if (json_array_append_new(arr, str) < 0) {
                json_decref(str);
                json_decref(arr);

                strncpy(srv->err, strerror(errno), sizeof(srv->err));
                srv->err[sizeof(srv->err) - 1] = '\0';

                json_t *response = json_pack("{siss}", "status", -1, "error", strerror(errno));
                if (response != NULL) {
                    dime_socket_push(&clnt->sock, response, NULL, 0);
                    json_decref(response);
                }

                return -1;
            }
        }
    }

    json_t *response = json_pack("{siso}", "status", 0, "devices", arr);
    if (response == NULL) {
        json_decref(arr);

        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    if (dime_socket_push(&clnt->sock, response, NULL, 0) < 0) {
        json_decref(response);

        strncpy(srv->err, strerror(errno), sizeof(srv->err));
        srv->err[sizeof(srv->err) - 1] = '\0';

        response = json_pack("{siss}", "status", -1, "error", strerror(errno));
        if (response != NULL) {
            dime_socket_push(&clnt->sock, response, NULL, 0);
            json_decref(response);
        }

        return -1;
    }

    json_decref(response);

    return 0;
}
