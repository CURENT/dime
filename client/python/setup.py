#!/usr/bin/env python3
# See https://packaging.python.org/tutorials/packaging-projects/ for reference

from setuptools import setup

with open("README.md", "r") as file:
    long_description = file.read()

setup(
    name = "dime-client",
    version = "0.0.1",
    description = "DiME client for Python",
    long_description = long_description,
    long_description_content_type = "text/markdown",
    author = "Zack Malkmus",
    author_email = "zmalkmus@vols.utk.edu",
    license = "GNU License V3",
    url = "https://github.com/zmalkmus/dimedev",
    packages = ["dime"],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    install_requires = [
        "numpy",
    ]
)
