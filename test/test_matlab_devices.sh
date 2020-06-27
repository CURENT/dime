#!/bin/sh -e

printf "Running test_matlab_devices... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

env MATLABPATH="../client/matlab" matlab -batch "test_matlab_devices('$DIME_SOCKET')"

kill $DIME_PID

printf "Done!\n"