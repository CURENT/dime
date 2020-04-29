#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    size_t n;

    if (nrhs != 2) {
        mexErrMsgTxt("Wrong number of arguments (given %d, expected 2)", nrhs);
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = mxGetInt64s(prhs[0])[0];
    n = mxGetM(prhs[1]) * mxGetN(prhs[1]) * mxGetElementSize(prhs[1]);

    n = send(fd, mxGetData(prhs[1]), n, 0);
    if (n < 0) {
        mexErrMsgTxt("send: %s", strerror(errno));
    }
}
