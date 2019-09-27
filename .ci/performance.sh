#!/bin/bash
set -e -x

python -m pip install -U -r dev-requirements.txt
python setup.py install
python cbitstruct/tests/test_perf.py
