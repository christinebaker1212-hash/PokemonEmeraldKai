#!/bin/bash
# Moemon Maker M4b, "Start from the game's script" (src/maker/maker_scripts.c, tools/maker/decompile.py --pack):
# every person's, sign's and trigger's script as a Maker event compiles; the pack's event for a sign is the same as
# decompile.py's; and in LITTLEROOT TOWN a sign given the event of its own script (its words changed) and a sign given
# its script unchanged both run (the game reads them as it reads its own).
# usage: tools/test/maker_gamescript.sh [BIN]
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_gamescript_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
# the lab sign with new words, the town sign as the game has it (each logs when it runs)
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
event LABSIGN on=sign:LittlerootTown_EventScript_BirchsLabSign@6,17
  page trigger=talk
    label LittlerootTown_EventScript_BirchsLabSign
    log 1
    raw msgbox "MAKER'S LAB: the game's script, new words", MSGBOX_SIGN
    raw end
S
python3 "$ROOT/tools/maker/decompile.py" "$ROOT/data/maps/LittlerootTown" 2>/dev/null \
  | awk '/^event LittlerootTown_EventScript_TownSign$/{p=1; print "event TOWNSIGN on=sign:LittlerootTown_EventScript_TownSign@15,13"; next}
         /^event /{p=0} p' | sed 's/^\(    label .*\)$/\1\n    log 2/' >> "$T/mm/base/MAP_LITTLEROOT_TOWN.txt"
python3 "$ROOT/tools/maker/decompile.py" "$ROOT/data/maps/LittlerootTown" 2>/dev/null \
  | awk '/^event LittlerootTown_EventScript_BrendansHouseSign$/{p=1; next} /^event /{p=0} p' | sed 's/^  //' > "$T/want_script.txt"
cat > "$T/play.txt" <<'S'
pressuntil A field 5000
wait 20
mp makerpackcheck
mp makergamescript LittlerootTown_EventScript_BrendansHouseSign
mp face up
wait 10
pressuntillog A 300 maker event: line 1 ran
wait 60
shot labsign
mash A 6 20
wait 30
mp warp 0,9,15,14
wait 60
mp face up
wait 10
pressuntillog A 300 maker event: line 2 ran
wait 60
shot townsign
mash A 6 20
wait 30
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,6,18 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 600 "$BIN" > "$T/play.log" 2>&1)
want() { grep -q -- "$1" "$T/play.log" || { echo "FAIL: no '$1' in play.log"; fail=1; }; }
want "maker pack: [0-9]* scripts, 0 don't compile"
grep "maker pack: .*: " "$T/play.log" | head -10
want "maker event: MAP_LITTLEROOT_TOWN LABSIGN compiled (1 page)"
want "maker event: MAP_LITTLEROOT_TOWN TOWNSIGN compiled (1 page)"
want "maker event: line 1 ran"
want "maker event: line 2 ran"
grep "\[mp\] maker script: " "$T/play.log" | sed 's/^.*\[mp\] maker script: //' > "$T/got_script.txt"
diff -q <(tail -n +2 "$T/want_script.txt") <(tail -n +2 "$T/got_script.txt") > /dev/null || { echo "FAIL: the pack's event differs from decompile.py's"; diff <(tail -n +2 "$T/want_script.txt") <(tail -n +2 "$T/got_script.txt") | head; fail=1; }
grep -q "left out\|maker event: .*: line" "$T/play.log" && { echo "FAIL: an edit or event was left out"; grep "left out\|maker event: .*: line" "$T/play.log"; fail=1; }
# the editor: the lab sign picked shows "Start from the game's script"; its test command makes the event and SAVE
# writes it to the map's edit file
E="$T/editor"; mkdir -p "$E/mm"
printf 'AllowSmallTweaks = 1\n' > "$E/on.ini"
cat > "$E/play.txt" <<'S'
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
maker mode 2
maker pick 6 17
wait 5
makershot picked
wait 10
maker eventgame LABSIGN sign:LittlerootTown_EventScript_BirchsLabSign@6,17 LittlerootTown_EventScript_BirchsLabSign
wait 5
makershot event
maker save
wait 5
quit
S
(cd "$E" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$E/play.txt" \
    MOEMON_SHOTDIR="$E" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$E/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$E/mm" timeout 200 "$BIN" > "$E/play.log" 2>&1)
for n in picked event; do grep -q "\[maker\] shot .*$n.bmp ok" "$E/play.log" || { echo "FAIL: no editor picture $n"; fail=1; }; done
grep -q "^    label LittlerootTown_EventScript_BirchsLabSign" "$E/mm/base/MAP_LITTLEROOT_TOWN.txt" 2>/dev/null \
    || { echo "FAIL: the event wasn't saved from the game's script"; fail=1; }
[ $fail = 0 ] && echo "maker_gamescript: OK"
exit $fail
