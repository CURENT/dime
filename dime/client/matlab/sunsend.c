#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    size_t n;
    ssize_t m;
    unsigned char *data;

    void (*handler)(int);

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
    data = mxGetData(prhs[1]);

    handler = signal(SIGPIPE, SIG_IGN);

    while (n > 0) {
        m = send(fd, data, n, 0);

        if (m < 0) {
            signal(SIGPIPE, handler);
            mexErrMsgTxt(strerror(errno));
        }

        data += m;
        n -= m;
    }

    signal(SIGPIPE, handler);
}
