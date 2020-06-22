#!/bin/sh -e

printf "Running test_python_send... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

python3 test_python_send.py "$DIME_SOCKET"

kill $DIME_PID

printf "Done!\n"
