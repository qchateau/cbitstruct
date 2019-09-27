#!/bin/bash
set -e -x

python -m pip install -U pip twine wheel setuptools
python -m twine upload --repository-url "$PYPI_REPOSITORY_URL" -u "$PYPI_USERNAME" -p "$PYPI_PASSWORD" dist/*
