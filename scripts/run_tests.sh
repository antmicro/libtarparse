#!/usr/bin/env bash
set -e -o pipefail

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
BASEDIR="$SCRIPTDIR/../"
TESTBUILDDIR="$BASEDIR/build/"
TESTWORKDIR="$BASEDIR/src/test/"
echo "Script directory: $SCRIPTDIR"

# Create build directory
mkdir -v -p $TESTBUILDDIR
pushd $TESTBUILDDIR
cmake ../ -DTARPARSE_COMPILE_TESTS=
make
popd

# Run tests in the proper working directory
pushd $TESTWORKDIR
RC=0
$TESTBUILDDIR/tarparse_tests || RC=$?
popd

exit $RC

