#!/bin/bash
set -e -x

export COVERAGE=1

python -m pip install -U -r dev-requirements.txt
python -m pip install -U cpp-coveralls
python setup.py install

# move to a different directory to avoid picking-up the local files
mkdir ci-coverage && cd ci-coverage
python -m nose2 cbitstruct
coveralls --exclude clinic --gcov-options '\-lp'
