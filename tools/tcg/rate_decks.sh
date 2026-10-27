#!/bin/bash
# CARD SCALING's deck ratings: src/tcg/tcg_ratings.c (tools/tcg/rate_decks.c). Run it again
# whenever the rules, the AI or a deck change (tools/tcg/test.sh checks it's up to date).
# About a minute.
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT=$(mktemp -d /tmp/moemon-tcg-rate.XXXXXX)
trap 'rm -rf "$OUT"' EXIT
gcc -std=c99 -O2 -Wall -Wextra -Werror -DTCG_TEST -DTCG_RATING_TOOL -DMOEMON_DEV_BUILD=0 -iquote include \
    tools/tcg/rate_decks.c src/tcg/tcg_scale.c src/tcg/tcg_decks.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c \
    src/tcg/tcg_catalog.c src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -lm -o "$OUT/rate"
"$OUT/rate" > "$OUT/tcg_ratings.c"
mv "$OUT/tcg_ratings.c" src/tcg/tcg_ratings.c
echo "src/tcg/tcg_ratings.c: $(grep -c '/\*' src/tcg/tcg_ratings.c) ratings"
