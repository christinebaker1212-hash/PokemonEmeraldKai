#!/bin/bash
# The Magic engine's tests, built natively with the sanitizers: the card pack read whole, set-up tables for the
# translated rules, and whole duels between the stand-in AIs (they end; the same seed plays the same duel).
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/mtg-tests}
mkdir -p "$OUT"
gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-sanitize-recover=undefined \
    -iquote include -iquote src/mtg tools/mtg/test_rules.c src/mtg/*.c -o "$OUT/test_rules"
"$OUT/test_rules" graphics/mtg/forge.pak "$@"
