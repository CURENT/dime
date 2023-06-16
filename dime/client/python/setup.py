#!/usr/bin/env python3
# See https://packaging.python.org/tutorials/packaging-projects/ for reference

from setuptools import setup

with open("README.md", "r") as file:
    long_description = file.read()

setup(
    name = "dime",
    version = "0.0.1",

    description = "DiME client for Python",
    long_description = long_description,
    long_description_content_type = "text/markdown",

    author = "Nicholas West",
    author_email = "nwest13@vols.utk.edu",

    url = "https://github.com/TheHashTableSlasher/dime2",
    packages = ["dime"]
)
