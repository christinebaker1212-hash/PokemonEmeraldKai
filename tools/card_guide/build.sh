#!/bin/bash
# The card guide (docs/CARD_GUIDE.html): every card of the three card games and where it comes from, read from the
# game's own tables (the catalogs, packs, shops, duelists' decks, special trainers). Rerun after card, deck, shop or
# reward changes:  tools/card_guide/build.sh
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/card-guide}
mkdir -p "$OUT"
gcc -std=gnu99 -w -DTCG_TEST -DMOEMON_DEV_BUILD=0 -iquote include -Isrc/tcg tools/card_guide/dump_tcg.c \
    src/tcg/tcg_catalog.c src/tcg/tcg_collection.c src/tcg/tcg_decks.c -o "$OUT/dump_tcg"
"$OUT/dump_tcg" > "$OUT/tcg.json"
# (Yu-Gi-Oh!'s catalog needs EDOPro's card data: the engine library the ratings tools build)
tools/ygo/edo/build_core.sh "$OUT" > /dev/null
gcc -std=gnu99 -O1 -w -iquote include -iquote src/ygo tools/card_guide/dump_ygo.c src/ygo/ygo_catalog.c \
    src/ygo/ygo_collection.c src/ygo/ygo_fusions.c src/ygo/ygo_edo.c src/ygo/ygo_edo_data.c src/ygo/ygo_edo_pak.c \
    src/ygo/ygo_edo_codes.c tools/ygo/stb_zlib.c -L"$OUT" -locgcore -lstdc++ -lm -o "$OUT/dump_ygo"
"$OUT/dump_ygo" 2>/dev/null | grep -v '^\[' | grep . > "$OUT/ygo.json"
gcc -std=gnu99 -w -iquote include tools/card_guide/dump_tt.c src/tt/tt_catalog.c src/tt/tt_pools.c -o "$OUT/dump_tt"
"$OUT/dump_tt" > "$OUT/tt.json"
python3 tools/card_guide/gen_guide.py "$OUT" docs/CARD_GUIDE.html
