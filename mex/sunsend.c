#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <stdio.h>

#include "mex.h"

static void brokenpipe(int sigtype) {
    mexErrMsgTxt(strerror(EPIPE));
}

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    ssize_t n;
    void (*sighandle)(int);

    if (nrhs != 2) {
        mexErrMsgTxt("Wrong number of arguments");
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = ((mxInt64 *)mxGetData(prhs[0]))[0];
    n = mxGetM(prhs[1]) * mxGetN(prhs[1]) * mxGetElementSize(prhs[1]);

    sighandle = signal(SIGPIPE, brokenpipe);
    if (sighandle == SIG_ERR) {
        mexErrMsgTxt(strerror(errno));
    }

    n = send(fd, mxGetData(prhs[1]), n, 0);
    if (n < 0) {
        signal(SIGPIPE, sighandle);
        mexErrMsgTxt(strerror(errno));
    } else if (n == 0) {
        signal(SIGPIPE, sighandle);
        mexErrMsgTxt("Socket closed by server");
    }

    signal(SIGPIPE, sighandle);
}
