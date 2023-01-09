#!/usr/bin/env bash
set -e -o pipefail

# Get the directory the script is placed in, to allow running it from anywhere
SCRIPTDIR=$(dirname "$(readlink -f "$0")")
BASEDIR="$SCRIPTDIR/../"
WORKDIR="$BASEDIR/build-release"

mkdir -v -p "$WORKDIR"
cd "$WORKDIR"

HEADY_URL="https://github.com/JamesBoer/Heady.git"
HEADY_COMMIT_HASH="876f730a30b4815ba6f657f222aaca23bfdc360f"

# Clone and compile the Heady utility
# This is used for generating a single-header version of the library
if [ ! -d heady/ ]; then
	git clone "$HEADY_URL" heady/
	(cd ./heady && git checkout "$HEADY_COMMIT_HASH")
fi
pushd ./heady/
(cd ./Bin && ./BuildMakefiles.sh)
cp -v ./Build/Release/Heady "$WORKDIR"
popd

# Copy the sources to the workdir and generate a single-header
HEADERPATH="$WORKDIR/tarparse.hpp"
cp -rv "$BASEDIR/src/include/tarparse" "$WORKDIR"
./Heady -r -s "tarparse" -o "$HEADERPATH"
echo "Generated single-header in $HEADERPATH"

