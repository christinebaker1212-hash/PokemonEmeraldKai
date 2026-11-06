#!/bin/bash
# pret/pokeplatinum at the commit SINNOH's 3D pack is built from, only the parts tools/sinnoh/build_pack.py reads (the
# map header table, map matrices, map blocks, areas, props and their textures, lights, the places' events (warps):
# about 60 MB) and build_gfx.py (the overworld sprites, the trainers' front pictures). The release runs
# it (.github/workflows/release.yml) and then build_pack.py; nothing from it is committed here.
# usage: tools/sinnoh/fetch_decomp.sh DIR   (DIR is made, or refreshed if an earlier run made it)
set -e
DIR="${1:?usage: fetch_decomp.sh DIR}"
COMMIT=6eeb1b04c6433761f3049811b7e1383619c59743   # (2026-10-24; change it only with a check that the pack still matches)
PARTS=24  # what is fetched (2: + res/field/events, the warps; 3: + the sprites and trainer pictures, build_gfx.py;
          # 4: + the TOWN MAP's art and grid, gen_townmap.py; 5: + the POKéTCH's art, build_poketch.py; 6: + the field effect
          # models, the gyms' bags, tire stacks and snowballs: gymprops.py; 7: + the briefcase's starter screen,
          # build_starter.py; 8: + the map textures' and props' animations, fldtanime and
          # bm_anime_list: build_pack.py; 9: + the words and the scripts: res/text (build_text.py), res/field/scripts
          # and the trainers' data (gen_scripts.py); 10: + the TV's schedule, res/prebuilt/arc/tv.narc (tv_data.py); 11: + the Seals' prices and shops
          # (tv_data.py seal_lines); 12: + SUPER CONTESTS' accessories and backdrops (build_contest.py); 13: + the contests' rules
          # tables, the moves' contest data and contest_data.narc (contest_data.py); 14: + DAWN's and LUCAS's own
          # overworld sheets (build_gfx.py), the Underground's digging (contest_data.py mining_lines); 15: + its art (build_mining.py); 16: + the UNOWN font (build_unown.py); 17: + the BERRY patches' first BERRIES (contest_data.py berry_lines); 18: all of generated/, whose
          # lists gen_scripts.py reads as constants (the roamers' slots, fade types...); 19-20: + the VILLA's furniture,
          # src/overlay005/villa_furniture.c and include/overlay005/villa_furniture.h, and the gyms' src/overlay008/
          # gym_features.c: gen_gyms.py, which also reads them); 21: + the BATTLE FRONTIER's own scripts,
          # res/field/frontier_scripts (gen_scripts.py FrontierConverter: the BATTLE TOWER's rooms); 22: + the buildings'
          # material / shape lists, build_model_matshp.dat (build_pack.py: drawn as Platinum draws them); 23: + the JOURNAL's
          # notebook, res/prebuilt/graphic/f_note_gra.narc (build_journal.py); 24: + the DISTORTION WORLD's floors,
          # res/prebuilt/fielddata/tornworld/ and its overlay's tables, src/overlay009/ (gen_distortion.py); a
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
git sparse-checkout set --no-cone /generated/ /include/data/map_headers.h /res/field/matrices/ \
    /res/field/maps/ /res/field/area_data/ /res/field/props/ /res/field/lighting/ /res/field/events/ \
    /res/graphics/field_sprites/npc/ /res/graphics/field_sprites/pokemon/ /res/graphics/field_sprites/objects/ \
    '/res/trainers/classes/*/front.png' /res/graphics/town_map/ /res/town_map/ \
    /res/graphics/poketch/ /res/graphics/field_effects/ /res/prebuilt/graphic/ev_pokeselect.narc \
    /res/prebuilt/data/fldtanime.narc /res/prebuilt/arc/bm_anime_list.narc \
    /res/text/ /res/field/scripts/ /res/trainers/ /include/constants/ /asm/macros/ \
    /res/prebuilt/arc/tv.narc /include/data/mart_items.h /src/applications/capsule_menu/main.c \
    /res/graphics/fashion_case/ \
    /res/moves/ /res/prebuilt/contest/ /src/unk_02094EDC.c /src/overlay017/ov17_02246ECC.c \
    /res/graphics/field_sprites/player/ /src/underground/mining.c /res/graphics/mining/ /res/fonts/font_unown.png /res/fonts/font_unown.json \
    /include/data/berry_init.h /src/overlay005/villa_furniture.c /include/overlay005/villa_furniture.h \
    /src/overlay008/gym_features.c /res/field/frontier_scripts/ /res/prebuilt/fielddata/build_model/build_model_matshp.dat \
    /res/prebuilt/graphic/f_note_gra.narc /res/prebuilt/fielddata/tornworld/ /res/prebuilt/data/tw_arc_etc.narc \
    /src/overlay009/ /include/constants/distortion_world.h
for i in 1 2 3 4; do
    git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && break
    sleep $((2 ** i))
done
git checkout -q FETCH_HEAD
echo "$MARK" > .s3d_commit
echo "pokeplatinum $COMMIT in $DIR"
