#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "mex.h"

/* Set a 200MB receiver buffer size */
#define BUFLEN 200000000

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    void *buf;
    ssize_t n;

    if (nrhs != 1) {
        mexErrMsgTxt("Wrong number of arguments (given %d, expected 1)", nrhs);
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = mxGetInt64s(prhs[0])[0];

    buf = malloc(BUFLEN);
    if (buf == NULL) {
        mexErrMsgTxt("malloc: %s", strerror(errno));
    }

    n = recv(fd, buf, BUFLEN, 0);
    if (n < 0) {
        free(buf);
        mexErrMsgTxt("recv: %s", strerror(errno));
    }

    plhs[0] = mxCreateNumericMatrix(1, n, mxUINT8_CLASS, 0);
    memcpy(mxGetData(plhs[0]), buf, n);
    free(buf);
}
