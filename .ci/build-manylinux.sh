#!/bin/bash
set -e -x

docker pull $DOCKER_IMAGE
docker run --rm -e PLAT=$PLAT -e PY_VERSIONS="$PY_VERSIONS" -v `pwd`:/io $DOCKER_IMAGE $PRE_CMD /io/.ci/build-manylinux-docker.sh
ls dist/
