#!/usr/bin/env bash
set -e -o pipefail

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
echo "Script directory: $SCRIPTDIR"

# Create build directory, generate instrumented binary
mkdir -v -p "$SCRIPTDIR/build/"
pushd "$SCRIPTDIR/build/"
export CC=afl-cc
export CXX=afl-c++
cmake ../ -DTARPARSE_COMPILE_FUZZ=
make
popd

pushd "$SCRIPTDIR/src/fuzz/"
afl-fuzz -i ./corpus -o ./output -- ../../build/tarparse_fuzzer
#pushd "$SCRIPTDIR/src/test/"
#RC=0
#../../build/tarparse_tests || RC=$?
#popd

exit $RC

