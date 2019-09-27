#!/bin/bash
set -e -x

export COVERAGE=1

python -m pip install -U -r dev-requirements.txt
python -m pip install -U cpp-coveralls
python setup.py install
python -m nose cbitstruct
coveralls --exclude clinic --gcov-options '\-lp'
