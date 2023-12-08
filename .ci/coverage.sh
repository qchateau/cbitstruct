#!/bin/bash
set -e -x

export COVERAGE=1

python -m pip install .[test]
python -m pip install -U cpp-coveralls

# Move to a different directory to avoid picking-up the local files.
# Alternatively, in Python 3.11+ use the interpreter's -P option to ignore local files.
mkdir ci-coverage && cd ci-coverage
python -m unittest discover cbitstruct
coveralls --exclude clinic --gcov-options '\-lp'
