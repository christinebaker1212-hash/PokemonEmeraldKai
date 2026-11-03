#!/bin/bash
# Moemon Maker's F5 (src/maker/maker_play.c, maker/maker_ui.cpp): played from a copy of save file 1 at a spot in
# LITTLEROOT TOWN; saving there writes only MoemonMaker/.play/play.sav (the real save unchanged, byte for byte); an
# edit while playing shows at once (a tile in place; a moved person by the map loading again); F12's stop goes back to
# editing with the real save. Then the start-of-game story point. Pictures in /tmp/maker_play_test.
# usage: tools/test/maker_play.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_play_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
run() {   # run NAME SCRIPT [VAR=value...]
    local name=$1 script=$2; shift 2
    (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/$script" \
        MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_DIR="$T/mm" "$@" \
        timeout 200 "$BIN" > "$T/$name.log" 2>&1)
}
printf 'pressuntil A field 5000\nwait 30\nmp save\nwait 60\nquit\n' > "$T/make.txt"
run make make.txt MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_NAME=REAL
REAL=$(ls "$T"/saves/slot1/pokeemerald.sav)
md5sum "$REAL" > "$T/real.md5"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_LITTLEROOT_TOWN\nwait 4\n'
    printf 'maker play 0 1 10 12\nwaitfor field 1500\nwait 60\nshot play\n'
    printf 'maker stamp 4\nmaker paint 9 11\nmaker paint 11 11\nwait 20\nshot hot_tiles\n'
    printf 'maker mode 2\nmaker pick 16 10\nmaker move 10 14\nwait 150\nwaitfor field 600\nwait 30\nshot hot_people\n'
    printf 'mp save\nwait 60\nmakershot editor_playing\nmaker stop\nwait 30\nmakershot editor_after\n'
    printf 'maker play 1 1 10 12\nwaitfor field 2000\nwait 60\nshot start\nmp checkitem 4\nmp save\nwait 60\nmaker stop\nwait 30\nquit\n'
} > "$T/play.txt"
run play play.txt MOEMON_MAKER_UNLOCK=1
want() { grep -q -- "$1" "$T/play.log" || { echo "FAIL: no '$1'"; fail=1; }; }
want "\[maker\] playing MAP_LITTLEROOT_TOWN at 10, 12 (a copy of a save)"
want "\[maker\] back to editing"
want "\[maker\] playing MAP_LITTLEROOT_TOWN at 10, 12 (a story point)"
want "\[test\] item 4: 10"   # (the story point's 10 POKé BALLS)
md5sum -c "$T/real.md5" > /dev/null 2>&1 || { echo "FAIL: the real save changed"; fail=1; }
[ -s "$T/mm/.play/play.sav" ] || { echo "FAIL: no play save"; fail=1; }
cmp -s "$T/shots/play.bmp" "$T/shots/hot_tiles.bmp" && { echo "FAIL: the tile edit didn't show while playing"; fail=1; }
cmp -s "$T/shots/hot_tiles.bmp" "$T/shots/hot_people.bmp" && { echo "FAIL: the moved person didn't show while playing"; fail=1; }
[ $fail = 0 ] && echo "maker_play: OK"
exit $fail
