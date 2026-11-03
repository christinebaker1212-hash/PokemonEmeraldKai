#!/bin/bash
# Moemon Maker's editing (maker/maker_mapview.cpp, maker_edits.cpp) and the game reading the edits back
# (src/maker/maker_overrides.c): in LITTLEROOT TOWN a stamp painted, a rectangle and a fill, a tile blocked in Collision
# mode, the twin moved and given another look; an undo and a redo; Ctrl+S writes MoemonMaker/base/MAP_LITTLEROOT_TOWN.txt
# with each edit's old and new value; a game started there then has them (its tiles, the twin where she was moved).
# Pictures in /tmp/maker_edit_test.
# usage: tools/test/maker_edit.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_edit_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
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
maker open MAP_LITTLEROOT_TOWN
wait 4
maker stamp 4
maker paint 9 10
maker paint 10 11
maker stamp 4 2 1
maker rect 2 15 5 15
maker stamp 4
maker fill 14 2
maker undo
maker redo
maker mode 1
maker coll 1
maker paint 10 12
maker mode 2
maker pick 16 10
maker move 12 10
maker prop look OBJ_EVENT_GFX_BOY_1
maker prop movement MOVEMENT_TYPE_FACE_DOWN
wait 4
maker problems
makershot edited
maker save
maker edits
wait 2
maker close
wait 60
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
F="$T/mm/base/MAP_LITTLEROOT_TOWN.txt"
want() { grep -q -- "$2" "$1" 2>/dev/null || { echo "FAIL: no '$2' in $(basename "$1")"; fail=1; }; }
want "$T/edit.log" "\[maker\] Saved 1 map"
want "$T/edit.log" "problems MAP_LITTLEROOT_TOWN: 0"
want "$F" "^map MAP_LITTLEROOT_TOWN 20 20"
want "$F" "^block 9 10 0x3001 0x3004"
want "$F" "^block 10 12 0x3001 0x3404"                  # (filled with flowers, then blocked: its elevation kept)
want "$F" "^block 2 15 "                                # (the rectangle)
want "$F" "^person LOCALID_LITTLEROOT_TWIN x 16 12"
want "$F" "^person LOCALID_LITTLEROOT_TWIN look OBJ_EVENT_GFX_TWIN OBJ_EVENT_GFX_BOY_1"
want "$F" "^person LOCALID_LITTLEROOT_TWIN movement MOVEMENT_TYPE_WANDER_AROUND MOVEMENT_TYPE_FACE_DOWN"
[ -e "$T/mm/.recovery/MAP_LITTLEROOT_TOWN.txt" ] && { echo "FAIL: the recovery file stayed after saving"; fail=1; }
# the game reads them: a quick start in LITTLEROOT TOWN, the edits used (nothing left out)
printf 'pressuntil A field 5000\nwait 60\nshot field\nquit\n' > "$T/play.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 150 "$BIN" > "$T/play.log" 2>&1)
want "$T/play.log" "\[maker\] 1 edited maps"
grep -q "left out" "$T/play.log" && { echo "FAIL: the game left edits out"; fail=1; }
[ $fail = 0 ] && echo "maker_edit: OK"
exit $fail
