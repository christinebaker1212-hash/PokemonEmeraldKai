#!/bin/bash
# The Magic engine's tests, built natively with the sanitizers: the card pack read whole, set-up tables for the
# translated rules, and whole duels between the stand-in AIs (they end; the same seed plays the same duel).
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/mtg-tests}
mkdir -p "$OUT"
# the rules and the collection only: plain C (the screens, the table and the game hooks need the game itself)
SRC="src/mtg/mtg_script.c src/mtg/mtg_valid.c src/mtg/mtg_mana.c src/mtg/mtg_game.c src/mtg/mtg_effects.c src/mtg/mtg_ai.c
     src/mtg/mtg_collection.c src/mtg/mtg_decks.c src/mtg/mtg_text.c"
gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-sanitize-recover=undefined \
    -iquote include -iquote src/mtg tools/mtg/test_rules.c $SRC -o "$OUT/test_rules"
"$OUT/test_rules" graphics/mtg/forge.pak "$@"
