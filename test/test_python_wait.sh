#!/bin/sh -e

printf "Running test_python_wait... "

DIME_SOCKET="`mktemp -u`"
../server/dime -l "unix:$DIME_SOCKET" &
DIME_PID=$!

env PYTHONPATH="../client/python" python3 test_python_wait.py "$DIME_SOCKET" "0" &
PYTHON_PID=$!
env PYTHONPATH="../client/python" python3 test_python_wait.py "$DIME_SOCKET" "1"
wait $PYTHON_PID

kill $DIME_PID

printf "Done!\n"
