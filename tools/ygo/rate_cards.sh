#!/bin/bash
# The Yu-Gi-Oh! power audit: each AI level against the best, and every card's worth (three copies in a plain deck
# against the deck without them; two such decks, the median and a strong Game Boy deck), blended with the Forbidden & Limited lists' history (GOAT's 2005 list first) and
# calibrated against the Game Boy decks' ratings: writes src/ygo/ygo_card_ratings.c and docs/audit_ygo.txt.
# Rerun after a change to the AI, the engine or the cards (after rate_npc_decks.sh).
#   tools/ygo/rate_cards.sh [GAMES_PER_CARD] [LEVEL_GAMES]
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/ygo-tests}
GAMES=${1:-60}
LEVELS=${2:-500}
JOBS=${JOBS:-$(nproc)}
mkdir -p "$OUT"
tools/ygo/edo/build_core.sh "$OUT"
python3 - "$OUT/npc_decks.txt" <<'PY'
import json, sys
decks = json.load(open('tools/ygo/npc_decks.json'))
open(sys.argv[1], 'w').write(''.join(' '.join(map(str, d['cards'])) + '\n' for d in decks))
PY
gcc -std=gnu99 -O2 -w -iquote include -iquote src/ygo tools/ygo/rate_cards.c src/ygo/ygo_catalog.c \
    src/ygo/ygo_fusions.c src/ygo/ygo_edo.c src/ygo/ygo_edo_data.c src/ygo/ygo_edo_pak.c src/ygo/ygo_edo_codes.c \
    src/ygo/ygo_ai.c tools/ygo/stb_zlib.c -L"$OUT" -locgcore -lstdc++ -lm -o "$OUT/rate_cards"
COUNT=$(grep -c '^    \[YGO_CARD_' src/ygo/ygo_catalog.c || true)
COUNT=${COUNT:-1509}
STEP=$(( (COUNT + JOBS) / JOBS + 1 ))
pids=()
for ((j = 0; j < JOBS; j++)); do
    from=$(( 1 + j * STEP )); to=$(( 1 + (j + 1) * STEP ))
    "$OUT/rate_cards" cards "$OUT/npc_decks.txt" $from $to "$GAMES" 2>/dev/null | grep '^card' > "$OUT/cards.$j.txt" &
    pids+=($!)
done
"$OUT/rate_cards" levels "$OUT/npc_decks.txt" "$LEVELS" 2>/dev/null | grep '^level' > "$OUT/levels.txt"
for p in "${pids[@]}"; do wait $p; done
cat "$OUT"/cards.*.txt > "$OUT/cards.txt"
"$OUT/rate_cards" limits 2>/dev/null | grep '^lists\|^limits' > "$OUT/limits.txt"
"$OUT/rate_cards" lflist GOAT 2>/dev/null | grep -v '^\[ygo\]' > "$OUT/goat.conf"
python3 tools/ygo/rate_cards.py "$OUT/cards.txt" "$OUT/levels.txt" "$OUT/limits.txt" "$OUT/goat.conf"
