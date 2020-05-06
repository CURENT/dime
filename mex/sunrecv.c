#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    size_t n;
    ssize_t m;
    mxArray *buf;

    if (nrhs != 2) {
        mexErrMsgTxt("Wrong number of arguments");
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = ((mxInt64 *)mxGetData(prhs[0]))[0];
    n = (size_t)mxGetScalar(prhs[1]);

    buf = mxCreateNumericMatrix(1, n, mxUINT8_CLASS, 0);

    m = recv(fd, mxGetData(buf), n, MSG_WAITALL);
    if (m < 0) {
        mexErrMsgTxt(strerror(errno));
    } else if (m < n) {
        mexErrMsgTxt("Socket closed by server");
    }

    plhs[0] = buf;
}
