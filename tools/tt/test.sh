#!/bin/bash
# Triple Triad tests, built natively with the sanitizers: the rules, the computer player, the collection, the regions.
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/tt-tests}
mkdir -p "$OUT"
SRC="src/tt/tt_catalog.c src/tt/tt_common.c src/tt/tt_duel.c src/tt/tt_ai.c src/tt/tt_collection.c src/tt/tt_hands.c src/tt/tt_pools.c"
for t in collection hands duel; do
    gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-sanitize-recover=undefined \
        -iquote include -iquote src/tt tools/tt/test_$t.c $SRC -o "$OUT/test_$t"
    "$OUT/test_$t" "$@"
done
