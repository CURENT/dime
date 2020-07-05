#!/bin/sh -e

printf "Running test_python_send... \n"

DIME_SOCKET="`mktemp -u`"
../server/dime -vvv -f "$DIME_SOCKET" &
DIME_PID=$!

env PYTHONPATH="../client/python" python3 test_python_send.py "$DIME_SOCKET"

kill $DIME_PID

printf "Done!\n"
