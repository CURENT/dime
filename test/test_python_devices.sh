#!/bin/sh -e

printf "Running test_python_devices... "

DIME_SOCKET="`mktemp -u`"
../server/dime -f "$DIME_SOCKET" &
DIME_PID=$!

env PYTHONPATH="../client/python" python3 test_python_devices.py "$DIME_SOCKET"

kill $DIME_PID

printf "Done!\n"
