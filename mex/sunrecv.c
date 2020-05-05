#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <stdio.h>

#include "mex.h"

/* Set a 200MB receiver buffer size */
#define BUFLEN 200000000

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    ssize_t n;
    void *buf;

    if (nrhs != 1) {
        mexErrMsgTxt("Wrong number of arguments");
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = ((mxInt64 *)mxGetData(prhs[0]))[0];

    buf = malloc(BUFLEN);
    if (buf == NULL) {
        mexErrMsgTxt(strerror(errno));
    }

    n = recv(fd, buf, BUFLEN, 0);

    if (n < 0) {
        free(buf);
        mexErrMsgTxt(strerror(errno));
    } else if (n == 0) {
        free(buf);
        mexErrMsgTxt("Socket closed by server");
    }

    plhs[0] = mxCreateNumericMatrix(1, n, mxUINT8_CLASS, 0);
    memcpy(mxGetData(plhs[0]), buf, n);
    free(buf);
}
