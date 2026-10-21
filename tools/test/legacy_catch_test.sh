#!/bin/bash
# GENESIS / EPOCH BALL catch math against the original routines (see legacy_catch_test.c). usage: legacy_catch_test.sh [quick]
cd "$(dirname "$0")" || exit 1
OUTBIN="${TMPDIR:-/tmp}/legacy_catch_test.$$"
gcc -O2 -Wall -Wextra -Wno-unused-function -o "$OUTBIN" legacy_catch_test.c || exit 1
"$OUTBIN" "$@"; rc=$?
rm -f "$OUTBIN"
exit $rc
