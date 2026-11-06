#!/bin/bash
# Moemon Maker M2b's new maps (src/maker/maker_newmaps.c, maker/maker_world.cpp): the editor makes MAP_MY_TOWN from the
# New Town template (MoemonMaker/maps/MAP_MY_TOWN.txt) and the World view opens; then a game started inside the new
# map (group 47) loads it, with an added person whose event runs, and its own picture.
# usage: tools/test/maker_newmap.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_newmap_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/edit.txt" <<'S'
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
maker newmap MY_TOWN 24 18 2
wait 6
makershot newmap
maker world 0
wait 6
makershot world
maker close
wait 60
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
want() { grep -q -- "$2" "$1" 2>/dev/null || { echo "FAIL: no '$2' in $(basename "$1")"; fail=1; }; }
F="$T/mm/maps/MAP_MY_TOWN.txt"
want "$T/edit.log" "\[maker\] newmap MY_TOWN: [0-9]"
want "$F" "^newmap MAP_MY_TOWN 24 18"
want "$F" "^tilesets General Petalburg"
want "$F" "^row 17 "
# the game: a person with an event on the new map, started inside it
cat > "$T/mm/base/MAP_MY_TOWN.txt" <<'S'
map MAP_MY_TOWN 24 18
add person 1 elevation=3 look=OBJ_EVENT_GFX_GIRL_1 movement=MOVEMENT_TYPE_FACE_DOWN script=event:HELLO x=10 y=8
event HELLO on=added
  page trigger=talk
    log 7
    text "Welcome to my town!"
S
printf 'pressuntil A field 5000\nwait 60\nmp where\nshot inside\nmp face up\nwait 10\npressuntillog A 600 maker event: line 7 ran\nmash A 4 20\nwait 20\nquit\n' > "$T/play.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=47,0,10,9 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 150 "$BIN" > "$T/play.log" 2>&1)
want "$T/play.log" "\[maker\] 1 new map"
want "$T/play.log" "where map 47.0"
want "$T/play.log" "maker event: line 7 ran"
grep -q "\[crash\]\|left out" "$T/play.log" && { echo "FAIL: a crash or an edit left out"; fail=1; }
[ $fail = 0 ] && echo "maker_newmap: OK"
exit $fail
