# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import os
import sys
from setuptools import setup, Extension


extra_compile_args = []
extra_link_args = []
undef_macros = []


if sys.platform != "win32":
    extra_compile_args.extend(("-std=c11", "-Wall", "-Werror", "-O3"))


if os.environ.get("COVERAGE"):
    extra_compile_args.extend(("-g", "-O0", "-fprofile-arcs", "-ftest-coverage"))
    extra_link_args.append("-fprofile-arcs")
    undef_macros.append("NDEBUG")


with open("README.md", "r") as fh:
    long_description = fh.read()


setup(
    name="cbitstruct",
    version="1.2.0",
    author="Quentin CHATEAU",
    author_email="quentin.chateau@gmail.com",
    license="MPL-2.0",
    url="https://github.com/qchateau/cbitstruct",
    description="Faster C implementation of bitstruct",
    long_description=long_description,
    long_description_content_type="text/markdown",
    python_requires=">=3.8",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
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
    extras_require={"test": ["bitstruct"]},
    ext_modules=[
        Extension(
            "cbitstruct._cbitstruct",
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
            sources=["cbitstruct/_cbitstruct.c"],
            include_dirs=["cbitstruct/"],
            undef_macros=undef_macros,
        )
    ],
    packages=["cbitstruct", "cbitstruct.tests"],
)
