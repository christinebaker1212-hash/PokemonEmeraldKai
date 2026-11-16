#!/bin/bash
# pret/pokeplatinum's interface art at the commit UI STYLE's PLATINUM was drawn from (docs/ui_styles/UI_STYLES.md): the
# healthboxes, fonts, windows, START, party, summary, BAG, OPTIONS and battle interface pictures and the move
# buttons' palettes (about 13 MB). tools/sinnoh/rom_pack.sh runs it before building sinnoh/ui_platinum.pak; nothing
# from it is committed here.
# usage: tools/ui_styles/fetch_platinum.sh DIR   (DIR is made, or refreshed if an earlier run made it)
set -e
DIR="${1:?usage: fetch_platinum.sh DIR}"
COMMIT=62bdaacdca6c56a6a396f3e9911e2f922b79bba8   # (the mock-ups' own; change it only with check_specs.py passing)
MARK="$COMMIT ui 1"
if [ -f "$DIR/.ui_commit" ] && [ "$(cat "$DIR/.ui_commit")" = "$MARK" ]; then
    echo "pokeplatinum's interface art $COMMIT already in $DIR"
    exit 0
fi
if [ -e "$DIR" ] && [ ! -f "$DIR/.ui_commit" ] && [ -n "$(ls -A "$DIR")" ]; then
    echo "$DIR exists and wasn't made by this script; pick another folder" >&2
    exit 1
fi
rm -rf "$DIR"
mkdir -p "$DIR"
cd "$DIR"
git init -q
git remote add origin https://github.com/pret/pokeplatinum.git
git sparse-checkout set --no-cone /res/fonts/ /res/graphics/bag/ /res/graphics/battle/ /res/graphics/options_menu/ \
    /res/graphics/party_menu/ /res/graphics/pokemon_summary_screen/ /res/graphics/scroll_prompts/ \
    /res/graphics/start_menu/ /res/graphics/windows/ /res/graphics/yes_no_touch_menu/ /src/overlay011/move_palettes.c
for i in 1 2 3 4; do
    git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && break
    echo "fetch failed (try $i)"; sleep $((i * 10))
done
git checkout -q FETCH_HEAD
echo "$MARK" > .ui_commit
