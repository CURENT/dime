#!/bin/bash -e

function start_spinner {
    set +m
    printf "%-36s" "    $1 "
    { while : ; do for X in '  •     ' '   •    ' '    •   ' '     •  ' '      • ' '     •  ' '    •   ' '   •    ' '  •     ' ' •      ' ; do echo -en "\b\b\b\b\b\b\b\b$X" ; sleep 0.1 ; done ; done & } 2>/dev/null
    spinner_pid=$!
}

function stop_spinner {
    { kill -9 $spinner_pid && wait; } 2>/dev/null
    set -m
    echo -en "\033[2K\r"
}

trap stop_spinner EXIT

spinner_pid=

printf "Building dime from source... This may take a while."

# Install dependencies
printf "\n"
start_spinner "Installing dependencies:"
sudo apt-get install -y build-essential autotools-dev autoconf libev-dev libtool &> /dev/null
stop_spinner
echo "    Installing dependencies:      ✓"

# Build openssl
start_spinner "Building openssl:"
(cd openssl* && chmod +x config Configure && ./config && make && sudo make install) &> /dev/null
stop_spinner
echo "    Building openssl:             ✓"

# Build zlib
start_spinner "Building zlib:"
(cd zlib* && chmod +x configure && ./configure && make && sudo make install) &> /dev/null
stop_spinner
echo "    Building zlib:                ✓"

# Build jansson
start_spinner "Building jansson:"
(cd jansson* && autoreconf -i --force && chmod +x configure && ./configure && make && sudo make install) &> /dev/null
stop_spinner
echo "    Building jansson:             ✓"

# Build dime
start_spinner "Building dime:"
(cd dime/server && make && sudo make install) &> /dev/null
(cd dime/client/python && python3 setup.py install) &> /dev/null
stop_spinner
echo "    Building dime:                ✓"

printf "\nBuild process completed successfully! \n\nYou can now run the server from the dime/server directory with 'dime &'.\n\n"