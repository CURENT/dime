#!/bin/sh -e

printf "Running test_python_sync... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

python3 test_python_sync.py "$DIME_SOCKET"

kill $DIME_PID

printf "Done!\n"
