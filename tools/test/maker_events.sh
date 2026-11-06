#!/bin/bash
# Moemon Maker M2's events (src/maker/maker_events.c, maker_overrides.c): an edit file gives LITTLEROOT TOWN an added
# person with an event of its own (two pages: the second once self switch A is on; a yes / no question, an item, a flag
# of the user's own, a choice of three) and an autorun event that runs once. A game started there plays them.
# usage: tools/test/maker_events.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_events_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
add person 1 elevation=3 look=OBJ_EVENT_GFX_BOY_1 movement=MOVEMENT_TYPE_FACE_DOWN script=event:GREETER x=10 y=12
event GREETER on=added
  page trigger=talk
    log 1
    text "Hello! I was made in the Maker."
    yesno "Would you like two POTIONS?"
    if yes
      log 2
      giveitem ITEM_POTION 2
      setflag GOT_MAKER_POTION
      self A on
    else
      log 3
    end
  page trigger=talk self=A flag=GOT_MAKER_POTION
    log 4
    text "Which color do you like?"
    choice "RED" "BLUE" "GREEN"
    if choice 2
      log 5
    else
      log 6
    end
event INTRO on=map
  page trigger=autorun noself=A
    log 9
    text "Welcome to LITTLEROOT, {PLAYER}!"
    self A on
S
cat > "$T/play.txt" <<'S'
pressuntil A field 5000
wait 20
mash A 8 20
wait 40
mp checkitem 13
mp face up
wait 10
pressuntillog A 900 maker event: line 2 ran
mash A 14 20
wait 80
mp checkitem 13
shot after_gift
pressuntillog A 600 maker event: line 4 ran
wait 240
press A 2
wait 60
shot choice
press DOWN 2
wait 10
press A 2
wait 70
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 200 "$BIN" > "$T/play.log" 2>&1)
want() { grep -q -- "$1" "$T/play.log" || { echo "FAIL: no '$1' in play.log"; fail=1; }; }
want "maker event: MAP_LITTLEROOT_TOWN GREETER compiled (2 pages)"
want "maker event: MAP_LITTLEROOT_TOWN INTRO compiled (1 page)"
for n in 9 1 2 4 5; do want "maker event: line $n ran"; done
grep -q "maker event: line 3 ran\|maker event: line 6 ran" "$T/play.log" && { echo "FAIL: the wrong branch ran"; fail=1; }
[ "$(grep -c 'maker event: line 9 ran' "$T/play.log")" = 1 ] || { echo "FAIL: the autorun ran more than once"; fail=1; }
a=$(grep -o "\[test\] item 13: [0-9]*" "$T/play.log" | head -1 | grep -o "[0-9]*$"); b=$(grep -o "\[test\] item 13: [0-9]*" "$T/play.log" | sed -n 2p | grep -o "[0-9]*$")
[ -n "$a" ] && [ -n "$b" ] && [ $((b - a)) = 2 ] || { echo "FAIL: the POTIONS weren't given ($a -> $b)"; fail=1; }
grep -q "left out\|maker event: .*: line" "$T/play.log" && { echo "FAIL: an edit or event was left out"; grep "left out\|maker event: .*: line" "$T/play.log"; fail=1; }
[ $fail = 0 ] && echo "maker_events: OK"
exit $fail
