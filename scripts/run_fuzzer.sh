#!/usr/bin/env bash
set -e -o pipefail

if [ ! which clang >/dev/null 2>&1 ] || [ ! which clang++ >/dev/null 2>&1 ]; then
    echo "Running the fuzzer requires clang to be installed"
    exit 1
fi

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
BASEDIR="$SCRIPTDIR/../"
FUZZBUILDDIR="$BASEDIR/build-fuzz/"
FUZZWORKDIR="$BASEDIR/src/fuzz/"
echo "Script directory: $SCRIPTDIR"

# Make sure the build directory exists
mkdir -v -p $FUZZBUILDDIR

# Compile fuzzing target
pushd $FUZZBUILDDIR
export CC=clang
export CXX=clang++
cmake ../ -DTARPARSE_COMPILE_FUZZ=
make
popd

# Run the fuzzer with the provided corpus
pushd $FUZZWORKDIR
RC=0
$FUZZBUILDDIR/tarparse_fuzzer corpus/ || RC=$?
popd

exit $RC
