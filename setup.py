#!/usr/bin/env python
import os
from distutils.core import setup, Extension


extra_compile_args = ["-std=c11"]
extra_link_args = []

if os.environ.get("COVERAGE"):
    extra_compile_args += ["-g", "-O0", "-fprofile-arcs", "-ftest-coverage"]
    extra_link_args += ["-fprofile-arcs"]


with open("README.md", "r") as fh:
    long_description = fh.read()


setup(
    name="cbitstruct",
    version="0.0.1",
    author="Quentin CHATEAU",
    author_email="quentin.chateau@gmail.com",
    license="GPLv3",
    url="https://github.com/qchateau/cbitstruct",
    description="Faster C implementation of bitstruct",
    long_description=long_description,
    long_description_content_type="text/markdown",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    keywords=[
        "bit",
        "pack",
        "unpack",
        "struct",
        "bitfield",
        "bit parsing",
        "bit unpack",
        "bit pack",
        "C",
    ],
    ext_modules=[
        Extension(
            "_cbitstruct",
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
            sources=["cbitstruct/_cbitstruct.c"],
        )
    ],
    packages=["cbitstruct", "cbitstruct.tests"],
)
