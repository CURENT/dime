@echo off
cd server
make
cd ..\client\python
python3 setup.py install
