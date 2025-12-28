#!/bin/bash

set -e
premake5 gmake
make -j$(nproc) $@
echo "Build OK"

./scripts/test-tmu.sh
echo "Testbed Examples Built OK"
