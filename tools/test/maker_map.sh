#!/bin/bash
# Moemon Maker's MAP panel (maker/maker_mapview.cpp, src/maker/maker_tiles.c, maker_objects.c): maps of HOENN, JOHTO and
# KANTO drawn from the game's own tiles; the tile animations running (two pictures of ROUTE 104's water differ); the
# behavior marks found by the game's own checks; people with their sprites, Events mode's markers and a picked person in
# the INSPECTOR. Pictures in /tmp/maker_map_test.
# usage: tools/test/maker_map.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_map_test
rm -rf "$T"; mkdir -p "$T"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/map.txt" <<'S'
pressuntil START mainmenu 3000
wait 40
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press A 2
wait 60
maker size 1280 800
maker open MAP_ROUTE101
wait 5
maker stats
makershot route101
wait 3
maker open MAP_ROUTE104
maker center 20 60
wait 5
makershot water_a
wait 31
makershot water_b
wait 3
maker open MAP_LITTLEROOT_TOWN
maker mode 2
maker pick 16 10
wait 5
makershot littleroot_events
wait 3
maker open MAP_JOHTO_NEW_BARK_TOWN
wait 4
maker stats
makershot new_bark
wait 3
maker open MAP_KANTO_PALLET_TOWN
wait 4
maker stats
makershot pallet
wait 3
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/map.txt" \
    MOEMON_SHOTDIR="$T" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    timeout 150 "$BIN" > "$T/map.log" 2>&1)
want() { grep -q -- "$1" "$T/map.log" || { echo "FAIL: no '$1'"; fail=1; }; }
for n in route101 water_a water_b littleroot_events new_bark pallet; do want "\[maker\] shot .*$n.bmp ok"; done
want "stats MAP_ROUTE101: .*class1=91 .*class10=12"   # (91 tall grass, 12 ledges facing south)
want "stats MAP_JOHTO_NEW_BARK_TOWN: .*class5=4"      # (its 4 doors)
want "stats MAP_KANTO_PALLET_TOWN: .*class5=3"
cmp -s "$T/water_a.bmp" "$T/water_b.bmp" && { echo "FAIL: ROUTE 104's water didn't move"; fail=1; }
[ $fail = 0 ] && echo "maker_map: OK"
exit $fail
