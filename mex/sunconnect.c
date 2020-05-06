#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "mex.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    int fd;
    struct sockaddr_un addr;
    char *filename;

    if (nrhs != 1) {
        mexErrMsgTxt("Wrong number of arguments");
    }

    filename = mxArrayToString(prhs[0]);
    if (filename == NULL) {
        mexErrMsgTxt("Invalid argument");
    }

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        mexErrMsgTxt(strerror(errno));
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    strncpy(addr.sun_path, filename, sizeof(addr.sun_path));
    addr.sun_family = AF_UNIX;
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        close(fd);
        mexErrMsgTxt(strerror(errno));
    }

    plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, 0);
    ((mxInt64 *)mxGetData(plhs[0]))[0] = fd;
}
