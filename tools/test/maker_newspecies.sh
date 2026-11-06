#!/bin/bash
# Moemon Maker M3b: a new POKéMON (MoemonMaker/database.txt `new species 1051 "SPARKIT" base=SPECIES_PIKACHU`) leads
# the party with its own stats and name, a wild one is fought and caught; slot 1052, which no pack fills, stays the
# "?" and SEALED. Pictures in /tmp/maker_newspecies_test.
# usage: tools/test/maker_newspecies.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_newspecies_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
cat > "$T/mm/database.txt" <<'D'
new species 1051 "SPARKIT" base=SPECIES_PIKACHU
  stats 45 70 40 110 60 50
  types TYPE_ELECTRIC TYPE_FAIRY
new move 848 "ZAPPER" base=MOVE_THUNDER_SHOCK
  power 70
  accuracy 0
new item 746 "BIG POTION" base=ITEM_POTION
  price 500
D
cat > "$T/play.txt" <<'S'
pressuntil A field 5000
wait 20
mp dbinfo 1051 848 746
mp dbinfo 1052 849 747
mp teach 0 848
mp item 746 2
mp sealed 1051
mp sealed 1052
mp item 1 3
mp savedata ball:last 1
mp wild 1051:40
pressuntil A action 3000
wait 10
shot battle
press A 2
wait 20
press A 2
wait 30
pressuntil B action 3000
wait 10
shot zapped
press L 2
pressuntillog A 6000 catch exp
pressuntil A field 9000
wait 100
mp party
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_PARTY=1051:20 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 200 "$BIN" > "$T/play.log" 2>&1)
want() { grep -q -- "$2" "$1" 2>/dev/null || { echo "FAIL: no '$2' in $(basename "$1")"; fail=1; }; }
want "$T/play.log" "db species 1051 SPARKIT stats 45 70 40 110 60 50 types 13 18"
want "$T/play.log" "db move 848 ZAPPER power 70 accuracy 0"
want "$T/play.log" "db item 746 BIG POTION price 500"
want "$T/play.log" "item 746 x2: ok"
want "$T/play.log" "screenshot .*battle.bmp"
want "$T/play.log" "sealed 1051 0 base 25"
want "$T/play.log" "sealed 1052 1 base 0"
want "$T/play.log" "\[test\] party: 1051(.*1051("
# the editor: a new POKéMON made from PIKACHU, its stats changed, the DATABASE window open on it
rm -rf "$T/mm2"; mkdir -p "$T/mm2"
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_LITTLEROOT_TOWN\nwait 4\n'
    printf 'maker newspecies BOLTIE SPECIES_PIKACHU\nmaker db species SPECIES_NEW_1051 stats 50 80 45 120 70 55\n'
    printf 'maker newmove ZAPPER MOVE_THUNDER_SHOCK\nmaker db move MOVE_NEW_848 power 70\nmaker newitem TONIC ITEM_POTION\n'
    printf 'maker database\nwait 6\nmakershot editor\nwait 3\nquit\n'
} > "$T/edit.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm2" MOEMON_SAVE="$T/edit.sav" timeout 200 "$BIN" > "$T/edit.log" 2>&1)
want "$T/edit.log" "\[maker\] new species BOLTIE 1051"
want "$T/mm2/database.txt" '^new species 1051 "BOLTIE" base=SPECIES_PIKACHU'
want "$T/mm2/database.txt" "^  stats 50 80 45 120 70 55$"
want "$T/mm2/database.txt" "^# next new species 1052"
want "$T/mm2/database.txt" '^new move 848 "ZAPPER" base=MOVE_THUNDER_SHOCK'
want "$T/mm2/database.txt" "^  power 70$"
want "$T/mm2/database.txt" '^new item 746 "TONIC" base=ITEM_POTION' 
want "$T/edit.log" "shot .*editor.bmp ok"
[ $fail = 0 ] && echo "maker_newspecies: OK"
exit $fail
