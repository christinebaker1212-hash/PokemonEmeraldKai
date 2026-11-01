#!/bin/bash
# pret/pokeplatinum at the commit SINNOH's 3D pack is built from, only the parts tools/sinnoh/build_pack.py reads (the
# map header table, map matrices, map blocks, areas, props and their textures, lights, the places' events (warps):
# about 60 MB) and build_gfx.py (the overworld sprites, the trainers' front pictures). The release runs
# it (.github/workflows/release.yml) and then build_pack.py; nothing from it is committed here.
# usage: tools/sinnoh/fetch_decomp.sh DIR   (DIR is made, or refreshed if an earlier run made it)
set -e
DIR="${1:?usage: fetch_decomp.sh DIR}"
COMMIT=6eeb1b04c6433761f3049811b7e1383619c59743   # (2026-10-24; change it only with a check that the pack still matches)
PARTS=3   # what is fetched (2: + res/field/events, the warps; 3: + the sprites and trainer pictures, build_gfx.py); a
          # folder fetched with other parts is fetched again
MARK="$COMMIT parts $PARTS"
if [ -f "$DIR/.s3d_commit" ] && [ "$(cat "$DIR/.s3d_commit")" = "$MARK" ]; then
    echo "pokeplatinum $COMMIT already in $DIR"
    exit 0
fi
if [ -e "$DIR" ] && [ ! -f "$DIR/.s3d_commit" ] && [ -n "$(ls -A "$DIR")" ]; then
    echo "$DIR exists and wasn't made by this script; pick another folder" >&2
    exit 1
fi
rm -rf "$DIR"
mkdir -p "$DIR"
cd "$DIR"
git init -q
git remote add origin https://github.com/pret/pokeplatinum.git
git sparse-checkout set --no-cone /generated/map_headers.txt /include/data/map_headers.h /res/field/matrices/ \
    /res/field/maps/ /res/field/area_data/ /res/field/props/ /res/field/lighting/ /res/field/events/ \
    /res/graphics/field_sprites/npc/ /res/graphics/field_sprites/pokemon/ /res/graphics/field_sprites/objects/ \
    '/res/trainers/classes/*/front.png'
for i in 1 2 3 4; do
    git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && break
    sleep $((2 ** i))
done
git checkout -q FETCH_HEAD
echo "$MARK" > .s3d_commit
echo "pokeplatinum $COMMIT in $DIR"
