#!/bin/sh
(cd server ; make && make install)
(cd client/python ; $PYTHON setup.py install)
