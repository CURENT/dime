#!/bin/bash -e

# Install dependencies
sudo apt-get install -y build-essential autotools-dev autoconf libev-dev libtool

# Build openssl
(cd openssl && ./config && make && sudo make install)

# Build zlib
(cd zlib-* && chmod +x configure && ./configure && make && sudo make install)

# Build jansson
(cd jansson-* && autoreconf -i --force && chmod +x configure && ./configure && make && sudo make install)

# Build dime
(cd dime/server && make && sudo make install)
(cd dime/client/python && python3 setup.py install)
