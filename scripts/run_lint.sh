#!/usr/bin/env bash

if ! which clang-tidy >/dev/null 2>&1; then
    echo "Clang-tidy is required for linting"
    exit 1
fi

SCRIPTDIR=$(dirname "$(readlink -f "$0")")
BASEDIR="$SCRIPTDIR/../"
echo "Script directory: $SCRIPTDIR"

exec find "$BASEDIR/src/include/" \
    -name "*.hpp" \
    -exec clang-tidy {} +
