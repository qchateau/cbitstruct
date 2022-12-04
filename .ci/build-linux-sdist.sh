#!/bin/bash
set -e -x

python -m pip install -U -r dev-requirements.txt
python setup.py sdist

# test wheel, move out of here otherwise pip thinks
# cbitstruct is already installed
mkdir -p testdir
cd testdir
python -m pip install cbitstruct --no-index -f ../dist/
python -m nose2 cbitstruct
cd ..
