#!/bin/bash
set -e -x

# Show gcc version
gcc --version

# Compile wheels
for PYVER in $PY_VERSIONS; do
    PYNUM=$(echo $PYVER | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.[0-9]\+/\1\2/')
    PYBIN=$(echo /opt/python/cp${PYNUM}*/bin)
    "${PYBIN}/pip" install -r /io/dev-requirements.txt
    "${PYBIN}/pip" wheel /io/ -w dist/
done

# Bundle external shared libraries into the wheels
for whl in dist/*.whl; do
    auditwheel repair "$whl" --plat $PLAT -w /io/dist/
done

# Install packages and test
for PYVER in $PY_VERSIONS; do
    PYNUM=$(echo $PYVER | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.[0-9]\+/\1\2/')
    PYBIN=$(echo /opt/python/cp${PYNUM}*/bin)
    "${PYBIN}/pip" install cbitstruct --no-index -f /io/dist
    (cd "$HOME"; "${PYBIN}/nosetests" cbitstruct)
done
