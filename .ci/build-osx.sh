#!/bin/bash
set -e -x

export PATH="/Users/travis/.pyenv/shims:${PATH}"

brew update > /dev/null
brew install openssl readline
brew outdated pyenv || brew upgrade pyenv
brew install pyenv-virtualenv

for PY_VERSION in $PY_VERSIONS; do
    pyenv install --skip-existing $PY_VERSION
    pyenv global $PY_VERSION

    python -m pip install -U pip
    python -m pip install -U -r dev-requirements.txt
    python --version
    python -m pip --version
    python setup.py bdist_wheel

    # test wheel, move out of here otherwise pip thinks
    # cbitstruct is already installed
    mkdir -p testdir
    cd testdir
    python -m pip install cbitstruct --no-index -f ../dist/
    python -m nose cbitstruct
    cd ..
done

ls dist/