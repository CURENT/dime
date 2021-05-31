#!/bin/bash -e
(cd server ; make && make install)
(cd client/python ; $PYTHON setup.py install)
