#!/bin/sh -e

printf "Running test_matlab_python_interop... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

env MATLABPATH="../client/matlab" matlab -batch "test_matlab_python_interop('$DIME_SOCKET')" &
MATLAB_PID=$!
env PYTHONPATH="../client/python" python3 test_matlab_python_interop.py "$DIME_SOCKET"
wait $MATLAB_PID

kill $DIME_PID

printf "Done!\n"
