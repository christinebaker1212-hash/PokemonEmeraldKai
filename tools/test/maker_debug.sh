#!/bin/bash
# Moemon Maker M4's debugger (src/maker/maker_events.c, maker/maker_debug_ui.cpp): a breakpoint on line 4 of an event
# of the user's own holds it there in a test game (line 2 ran, line 4 not yet), WATCH reads a flag the event set, STEP
# runs one line and holds at the next, CONTINUE runs it to the end. (The event's lines: 1 page, 2 log 1, 3 setflag,
# 4 log 2, 5 log 3, 6 log 4.)
# usage: tools/test/maker_debug.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_debug_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
add person 1 elevation=3 look=OBJ_EVENT_GFX_BOY_1 movement=MOVEMENT_TYPE_FACE_DOWN script=event:DEBUGME x=10 y=11
event DEBUGME on=added
  page trigger=talk
    log 1
    setflag DEBUG_SEEN
    log 2
    log 3
    log 4
S
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_LITTLEROOT_TOWN\nwait 4\n'
    printf 'maker break MAP_LITTLEROOT_TOWN DEBUGME 4\nmaker debugger\n'
    printf 'maker play 1 1 10 12\nwaitfor field 2000\nwait 60\nmp face up\nwait 10\n'
    printf 'pressuntillog A 600 maker debugger: held at\nwait 30\nmaker held\nmaker watch DEBUG_SEEN\nmakershot debugger\n'
    printf 'maker step\nwait 20\nmaker held\nmaker continue\nwait 60\nmaker held\nmaker stop\nwait 30\nquit\n'
} > "$T/play.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 200 "$BIN" > "$T/play.log" 2>&1)
want() { grep -q -- "$1" "$T/play.log" || { echo "FAIL: no '$1' in play.log"; fail=1; }; }
want "maker debugger: held at MAP_LITTLEROOT_TOWN DEBUGME line 4"
want "\[maker\] held at MAP_LITTLEROOT_TOWN DEBUGME line 4"
want "\[maker\] watch DEBUG_SEEN = 1"
want "\[maker\] held at MAP_LITTLEROOT_TOWN DEBUGME line 5"
want "\[maker\] nothing held"
want "maker event: line 4 ran"
# the order: "log 1" ran before the hold, "log 2" only after it
h=$(grep -n "debugger: held at .* line 4" "$T/play.log" | head -1 | cut -d: -f1)
l1=$(grep -n "maker event: line 1 ran" "$T/play.log" | head -1 | cut -d: -f1)
l2=$(grep -n "maker event: line 2 ran" "$T/play.log" | head -1 | cut -d: -f1)
[ -n "$h" ] && [ -n "$l1" ] && [ -n "$l2" ] && [ "$l1" -lt "$h" ] && [ "$l2" -gt "$h" ] || { echo "FAIL: lines ran out of order ($l1 $h $l2)"; fail=1; }
[ $fail = 0 ] && echo "maker_debug: OK"
exit $fail
