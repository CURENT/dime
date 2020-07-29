#!/bin/sh -e

printf "Running test_python_sync... "

DIME_SOCKET="`mktemp -u`"
../server/dime -l "unix:$DIME_SOCKET" &
DIME_PID=$!

env PYTHONPATH="../client/python" python3 test_python_sync.py "$DIME_SOCKET"

kill $DIME_PID

printf "Done!\n"
