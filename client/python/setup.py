#!/usr/bin/env python3
# See https://packaging.python.org/tutorials/packaging-projects/ for reference

from setuptools import setup

with open("README.md", "r") as file:
    long_description = file.read()

setup(
    name="dime-client",
    version="0.0.1",

    description="DiME client for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",

    author=["Nicholas West", "Nicholas Parsly", "Jinning Wang"],
    author_email=["nwest13@vols.utk.edu", "nparsly@vols.utk.edu", "jinninggm@gmail.com"],

    url="https://github.com/CURENT/dime",
    packages=["dime"]
)
