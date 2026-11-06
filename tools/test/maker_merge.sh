#!/bin/bash
# Moemon Maker M4c, the three-way merge (maker/maker_merge.cpp, src/maker/maker_overrides.c MakerApi_Stale): an edit
# file for LITTLEROOT TOWN as if the game had changed under three of its edits (a person's x whose WAS the game no
# longer has, an event on a person that's gone, a tile whose WAS isn't the tile there) and one that still fits. The
# editor's THE GAME CHANGED lists the three; KEEP MINE rewrites the first's WAS to the game's value now, TAKE THE GAME'S
# drops the other two; saved, the file has what was chosen and the game leaves nothing out.
# usage: tools/test/maker_merge.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_merge_test
rm -rf "$T"; mkdir -p "$T/mm/base"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
block 5 5 0x0999 0x0001
person LittlerootTown_EventScript_FatMan@12,13 x 99 14
person LittlerootTown_EventScript_FatMan@12,13 y 13 14
event GHOST on=person:LOCALID_NOBODY_HERE
  page trigger=talk
    text "Nobody."
S
cat > "$T/play.txt" <<'S'
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
wait 10
makershot merge
wait 10
maker merge list
maker merge keep 0
wait 5
maker merge list
maker merge take 1
wait 5
maker merge take 0
wait 5
maker merge list
maker save
wait 5
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 200 "$BIN" > "$T/play.log" 2>&1)
grep "\[maker\] merge" "$T/play.log"
lists=$(grep -o "\[maker\] merge: [0-9]*$" "$T/play.log" | grep -o "[0-9]*$" | tr '\n' ' ')
[ "$lists" = "3 2 0 " ] || { echo "FAIL: the merge lists were '$lists', not '3 2 0 '"; fail=1; }
grep -q "merge: person LittlerootTown_EventScript_FatMan@12,13 x 99 12 14 0" "$T/play.log" || { echo "FAIL: the person's x wasn't listed with the game's value now"; fail=1; }
grep -q "merge: event GHOST 0 0 0 1" "$T/play.log" || { echo "FAIL: the event on a person that's gone wasn't listed"; fail=1; }
grep -q "merge: block 5 5 2457 [0-9]* 1 0" "$T/play.log" || { echo "FAIL: the tile wasn't listed"; fail=1; }
f="$T/mm/base/MAP_LITTLEROOT_TOWN.txt"
grep -q "^person LittlerootTown_EventScript_FatMan@12,13 x 12 14$" "$f" || { echo "FAIL: KEEP MINE didn't take the game's value as WAS"; cat "$f"; fail=1; }
grep -q "^person LittlerootTown_EventScript_FatMan@12,13 y 13 14$" "$f" || { echo "FAIL: the edit that fit was lost"; fail=1; }
grep -q "^block 5 5\|GHOST" "$f" && { echo "FAIL: TAKE THE GAME'S didn't drop the edit"; fail=1; }
grep -q "\[maker\] shot .*merge.bmp ok" "$T/play.log" || { echo "FAIL: no picture"; fail=1; }
[ $fail = 0 ] && echo "maker_merge: OK"
exit $fail
