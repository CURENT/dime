#!/bin/bash -e

# Install dependencies
(apt-get install build-essential)
(apt-get install autotools-dev)
(apt-get install autoconf)
(apt-get install libev-dev)

# Build openssl
(cd openssl ; ./config && make && make install)

# Build zlib
(cd zlib ; ./configure && make && make install)

# Build jansson
(cd jansson ; autoreconf -i && ./configure && make && make install)

# Build dime
(cd dime/server ; make && make install)
(cd dime/client/python ; $PYTHON setup.py install)
