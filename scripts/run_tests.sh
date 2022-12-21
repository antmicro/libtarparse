#!/usr/bin/env bash
set -e -o pipefail

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
echo "Script directory: $SCRIPTDIR"

# Create build directory
mkdir -v -p "$SCRIPTDIR/build/"
pushd "$SCRIPTDIR/build/"
cmake ../ -DTARPARSE_COMPILE_TESTS=
make
popd

# Run tests in the proper working directory
pushd "$SCRIPTDIR/src/test/"
RC=0
../../build/tarparse_tests || RC=$?
popd

exit $RC

