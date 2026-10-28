#!/bin/bash
# Yu-Gi-Oh! cards with their rules written, overall and per set; `tools/ygo/progress.sh todo` lists the rest.
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/ygo-tests}
mkdir -p "$OUT"
gcc -std=gnu99 -O1 -w -iquote include -iquote src/ygo tools/ygo/progress.c src/ygo/ygo_catalog.c src/ygo/ygo_fusions.c \
    src/ygo/ygo_duel.c src/ygo/ygo_ai.c src/ygo/ygo_collection.c src/ygo/ygo_fx.c src/ygo/ygo_fx_*.c -o "$OUT/progress"
"$OUT/progress" "$@"
