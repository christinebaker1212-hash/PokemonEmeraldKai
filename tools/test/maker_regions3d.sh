#!/bin/bash
# Moemon Maker on ORRE and UNOVA (each needs its own pack: orre/orre3d.pak from tools/orre/make_pack.sh, unova/black3d.pak
# from tools/unova/build_pack.py; a region without its pack is skipped): the region's map listed under its own name in
# MAPS, shown by what each tile does (the pack's blocks, behavior in the id), a tile blocked in Collision mode and saved,
# and the INSPECTOR's 3D PREVIEW: ORRE's from its engine (o3d.c O3D_RenderPreview), UNOVA's the running game's own
# picture (maker_play.c Maker_PeekUnova: the game plays out of sight at the tile, then stops). Pictures in
# /tmp/maker_regions3d_test. usage: tools/test/maker_regions3d.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_regions3d_test
rm -rf "$T"; mkdir -p "$T"
fail=0 ran=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
# region MAP X Y (the blocked tile) EXTRA_WAIT
one() {
    local region=$1 map=$2 x=$3 y=$4 extra=$5 d="$T/$1"
    mkdir -p "$d/shots" "$d/mm"
    {
        printf 'pressuntil START mainmenu 3000\nwait 40\n'
        for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
        printf 'press A 2\nwait 60\nmaker size 1400 860\nmaker open %s\nwait 4\nmaker stats\nmaker find %s\n' "$map" "$region"
        printf 'maker zoom 1\nwait 2\nmaker center %d %d\nmaker look %d %d\n' "$x" "$y" "$x" "$y"
        printf 'maker mode 1\nmaker coll 2\nmaker paint %d %d\nmaker mode 0\nwait %d\nmakershot preview\nmaker save\nwait 2\nquit\n' "$x" "$y" "$extra"
    } > "$d/s.txt"
    (cd "$ROOT" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$d/s.txt" \
        MOEMON_SHOTDIR="$d/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
        MOEMON_MAKER_DIR="$d/mm" MOEMON_SAVE="$d/s.sav" timeout 240 "$BIN" > "$d/run.log" 2>&1)
    grep -q "stats $map: .*| behaviors: .*[0-9A-F][0-9A-F]=" "$d/run.log" || { echo "FAIL: $region: $map's tiles weren't read"; fail=1; }
    # (the pack's blocks carry behaviors: more than one kind on the map, not the layout's placeholder)
    [ "$(grep -o "stats $map: .*" "$d/run.log" | sed 's/.*behaviors://' | wc -w)" -ge 2 ] || { echo "FAIL: $region: one behavior only (the placeholder, not the pack's blocks)"; fail=1; }
    grep -q "^block $x $y 0x[0-9A-F]* 0x[0-9A-F]*" "$d/mm/base/$map.txt" 2>/dev/null || { echo "FAIL: $region: no collision edit"; fail=1; }
    grep -q "shot .*preview.bmp ok" "$d/run.log" || { echo "FAIL: $region: no picture"; fail=1; }
    ran=1
}
if [ -f "$ROOT/orre/orre3d.pak" ]; then one ORRE MAP_ORRE_PHENAC_CITY 48 107 10; else echo "maker_regions3d: ORRE skipped (no orre/orre3d.pak)"; fi
if [ -f "$ROOT/unova/black3d.pak" ]; then
    one UNOVA MAP_UNOVA_NUVEMATOWN 10 10 400
    grep -q "UNOVA preview: taken" "$T/UNOVA/run.log" || { echo "FAIL: UNOVA: the preview's picture wasn't taken"; fail=1; }
else echo "maker_regions3d: UNOVA skipped (no unova/black3d.pak)"; fi
[ $fail = 0 ] && [ $ran = 1 ] && echo "maker_regions3d: OK"
exit $fail
