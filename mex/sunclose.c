#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs != 1) {
        mexErrMsgTxt("Wrong number of arguments (given %d, expected 1)", nrhs);
    }

    if (mxGetClassID(prhs[0]) != mxINT64_CLASS ||
        mxGetM(prhs[0]) != 1 ||
        mxGetN(prhs[0]) != 1) {
        mexErrMsgTxt("Invalid argument");
    }

    if (close(mxGetInt64s(prhs[0])[0]) < 0) {
        mexErrMsgTxt("close: %s", strerror(errno));
    }
}
