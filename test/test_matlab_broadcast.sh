#!/bin/sh -e

printf "Running test_matlab_broadcast... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

matlab -batch "test_matlab_broadcast('$DIME_SOCKET')"

kill $DIME_PID

printf "Done!\n"