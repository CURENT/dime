#!/bin/sh -e

printf "Running test_javascript_devices... "

DIME_PORT=`python3 <<HEREDOC
import random
import socket

while True:
    port = random.randrange(1 << 10, 1 << 15)

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
            srv.bind(("", port))
    except OSError:
        pass
    else:
        break

print(port)
HEREDOC`

../server/dime -P ws -p "$DIME_PORT" &
DIME_PID=$!

cat ../client/javascript/dime.js test_javascript_devices.js | node - "localhost" "$DIME_PORT"

kill $DIME_PID

printf "Done!\n"

