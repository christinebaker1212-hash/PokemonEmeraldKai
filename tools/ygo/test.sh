#!/bin/bash
# Yu-Gi-Oh! rules tests, built natively with the sanitizers.
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/ygo-tests}
mkdir -p "$OUT"
SRC="src/ygo/ygo_catalog.c src/ygo/ygo_fusions.c src/ygo/ygo_duel.c src/ygo/ygo_ai.c src/ygo/ygo_fx.c src/ygo/ygo_fx_*.c"
for t in duel cards effects ai; do
    gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-sanitize-recover=undefined -DYGO_TRACE \
        -iquote include -iquote src/ygo tools/ygo/test_$t.c $SRC -o "$OUT/test_$t"
    "$OUT/test_$t"
done
