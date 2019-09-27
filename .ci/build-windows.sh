#!/bin/bash
set -e -x

BASE_PATH=$PATH

for PY_VERSION in $PY_VERSIONS; do
    PYNUM=$(echo $PY_VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.[0-9]\+/\1\2/')

    export PATH=/c/Python$PYNUM:/c/Python$PYNUM/Scripts:$BASE_PATH

    choco install python --version $PY_VERSION --allow-downgrade
    python -m pip install -U pip
    python -m pip install -U -r dev-requirements.txt

    python --version
    python -m pip --version

    python setup.py bdist_wheel

    # test the wheel, need to move our of the current dir
    # otherwise pip thinks cbitstruct is already installed
    mkdir -p testdir
    cd testdir
    python -m pip install cbitstruct --no-index -f ../dist/
    python -m nose cbitstruct
    python -m pip uninstall -y cbitstruct
    cd ..
done

ls dist/
