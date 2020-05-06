#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    size_t n;
    ssize_t m;

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

#ifdef MSG_NOSIGNAL
    m = send(fd, mxGetData(prhs[1]), n, MSG_NOSIGNAL);
#else
    m = send(fd, mxGetData(prhs[1]), n, 0);
#endif
    if (m < 0) {
        mexErrMsgTxt(strerror(errno));
    } else if (m < n) {
        mexErrMsgTxt("Socket closed by server");
    }
}
