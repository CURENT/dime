#!/bin/sh -e

printf "Running test_matlab_wait... "

DIME_SOCKET="`mktemp -u`"
../server/dime -l "unix:$DIME_SOCKET" &
DIME_PID=$!

env MATLABPATH="../client/matlab" matlab -batch "test_matlab_wait('$DIME_SOCKET', 0)" &
MATLAB_PID=$!
env MATLABPATH="../client/matlab" matlab -batch "test_matlab_wait('$DIME_SOCKET', 1)"
wait $MATLAB_PID

kill $DIME_PID

printf "Done!\n"
