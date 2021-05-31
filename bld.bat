@echo off
cd server
make
if errorlevel 1 exit 1
make install
if errorlevel 1 exit 1
cd ../client/python
"%PYTHON%" setup.py install
if errorlevel 1 exit 1
