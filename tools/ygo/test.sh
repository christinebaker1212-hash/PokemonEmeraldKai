#!/bin/bash
# Yu-Gi-Oh! tests, built natively with the sanitizers: the collection, the EDOPro engine's bridge (duels end, every
# card always in one place, determinism, rebuilding from answers), card scenarios on today's rules, the online link.
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/ygo-tests}
mkdir -p "$OUT"
tools/ygo/edo/build_core.sh "$OUT"
SRC="src/ygo/ygo_catalog.c src/ygo/ygo_fusions.c src/ygo/ygo_edo.c src/ygo/ygo_edo_data.c src/ygo/ygo_edo_pak.c
     src/ygo/ygo_edo_codes.c src/ygo/ygo_ai.c src/ygo/ygo_collection.c src/ygo/ygo_net.c tools/ygo/stb_zlib.c"
for t in collection edo edo_cards net; do
    gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -fsanitize=address,undefined \
        -fno-sanitize-recover=undefined -iquote include -iquote src/ygo tools/ygo/test_$t.c $SRC \
        -L"$OUT" -locgcore -lstdc++ -lm -o "$OUT/test_$t" 2>&1 | grep -v "stb_image.h" | grep -E "error|warning" || true
    "$OUT/test_$t"
done
