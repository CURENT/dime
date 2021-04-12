#!/bin/bash -o errexit
(cd server ; make && make install)
(cd client/python ; python3 setup.py install)
exit $?
