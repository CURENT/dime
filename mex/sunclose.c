#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;

    if (nrhs != 1) {
        mexErrMsgTxt("Wrong number of arguments");
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = ((mxInt64 *)mxGetData(prhs[0]))[0];

    if (shutdown(fd, SHUT_RDWR) < 0) {
        mexErrMsgTxt(strerror(errno));
    }

    if (close(fd) < 0) {
        mexErrMsgTxt(strerror(errno));
    }
}
