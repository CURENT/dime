#!/usr/bin/sudo bash

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

function handle_error {
    stop_spinner
    echo -e "\nError: $1"
    exit 1
}

trap stop_spinner EXIT
spinner_pid=

printf "Building dime... This may take a while."

# Install dependencies
printf "\n"
start_spinner "Installing dependencies:"

(
    apt update \
    && apt install -y --no-install-recommends \
        git \
        build-essential \
        libsuitesparse-dev \
        libopenblas-dev \
        libjansson-dev \
        libssl-dev \
        zlib1g-dev \
        libev-dev \
        python3-pip \
    && rm -rf /var/lib/apt/lists/*
) &> /dev/null

if [ $? -ne 0 ]; then
    handle_error "Failed to install dependencies."
fi

python3 -m pip install --upgrade pip setuptools wheel &> /dev/null

if [ $? -ne 0 ]; then
    handle_error "Failed to install dependencies."
fi

stop_spinner
echo "    Installing dependencies:      ✓"

# Build dime
start_spinner "Building dime:"
(cd dime/server && make && make install) &> /dev/null

if [ $? -ne 0 ]; then
    handle_error "Failed to build dime server."
fi

(cd dime/client/python && python3 setup.py install) &> /dev/null

stop_spinner
echo "    Building dime:                ✓"

printf "\nBuild process completed successfully! \n\nYou can now run the server from the terminal with 'dime -vv -l unix:/tmp/dime2 -l ws:8818'.\n\n"