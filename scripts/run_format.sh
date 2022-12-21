#!/usr/bin/env bash

if ! which clang-format >/dev/null 2>&1; then
    echo "Clang-format is required for formatting"
    exit 1
fi

#  Allow switching between reformat <-> verify for CI runs
OPTS=""
if [[ "$1" == "verify" ]]; then
    OPTS="-Werror -n"
elif [[ "$1" == "reformat" ]]; then
    OPTS="-i"
else
    echo "Usage: $0 [verify | reformat]"
    exit 1
fi
FORMATARGS="$OPTS --style=file"

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
BASEDIR="$SCRIPTDIR/../"
exec find "$BASEDIR/src/" \
    -name "*.*pp" \
    ! -name "catch.hpp" \
    -exec clang-format $FORMATARGS {} +
