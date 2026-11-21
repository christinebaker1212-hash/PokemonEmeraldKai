#!/bin/bash
# The generic duelists' Yu-Gi-Oh! decks from YGOPRODeck, start to end (the user, 2026-11-21: docs/YGO_DECISIONS.md
# "NPC decks from YGOPRODeck"; COOP_NOTES "YU-GI-OH!'s generic duelists' decks from YGOPRODeck"):
#   1. fetch_ygoprodeck_decks.py  every deck, format by format (hours: the server is slow deep in its listings;
#                                 resumes; skip with NOFETCH=1)
#   2. fetch_banlists.py          every Forbidden & Limited list since 2002 (tools/ygo/banlists.json)
#   3. text_power.py              every card's power from its text (tools/ygo/card_power.json)
#   4. build_npc_pool.py          the decks made playable, cut to their era's list, kept once
#   5. rate_pool.py ROUNDS        Swiss rounds of AI duels (resumes from ~/.cache/moemon/npc_pool_results.txt)
#   6. write_npc_pool.py          graphics/ygo/npc_pool.bin and src/ygo/ygo_card_ratings.c
#   tools/ygo/npc_pool.sh [ROUNDS]
set -e
cd "$(dirname "$0")/../.."
ROUNDS=${1:-12}
OUT=${OUT:-/tmp/ygo-tests}
mkdir -p "$OUT"
[ -n "${NOFETCH:-}" ] || python3 tools/ygo/fetch_ygoprodeck_decks.py
[ -f tools/ygo/banlists.json ] || python3 tools/ygo/fetch_banlists.py
python3 tools/ygo/text_power.py
python3 tools/ygo/build_npc_pool.py
tools/ygo/edo/build_core.sh "$OUT"
gcc -std=gnu99 -O2 -w -iquote include -iquote src/ygo tools/ygo/rate_pool_worker.c src/ygo/ygo_catalog.c \
    src/ygo/ygo_fusions.c src/ygo/ygo_edo.c src/ygo/ygo_edo_data.c src/ygo/ygo_edo_pak.c src/ygo/ygo_edo_codes.c \
    src/ygo/ygo_ai.c tools/ygo/stb_zlib.c -L"$OUT" -locgcore -lstdc++ -lm -o "$OUT/rate_pool_worker"
python3 tools/ygo/rate_pool.py "$ROUNDS"
python3 tools/ygo/write_npc_pool.py
