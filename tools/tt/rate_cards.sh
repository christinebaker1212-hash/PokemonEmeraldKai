#!/bin/bash
# Rates every Triple Triad card and computer level by games the computer plays against itself:
# writes src/tt/tt_ratings.c. Rerun after a change to the rules, the computer player or the cards.
# usage: tools/tt/rate_cards.sh [GAMES]
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/tt-rate}
mkdir -p "$OUT"
SRC="src/tt/tt_catalog.c src/tt/tt_common.c src/tt/tt_duel.c src/tt/tt_ai.c src/tt/tt_collection.c src/tt/tt_hands.c src/tt/tt_pools.c src/tt/tt_rating.c src/tt/tt_ratings.c"
gcc -std=gnu99 -O2 -iquote include -iquote src/tt tools/tt/rate_cards.c $SRC -lm -o "$OUT/rate_cards"
"$OUT/rate_cards" "${1:-30000}" > "$OUT/tt_ratings.c"
mv "$OUT/tt_ratings.c" src/tt/tt_ratings.c
echo "wrote src/tt/tt_ratings.c"
