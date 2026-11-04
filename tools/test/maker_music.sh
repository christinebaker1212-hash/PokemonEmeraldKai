#!/bin/bash
# Moemon Maker's music (maker/maker_music.cpp, src/maker/maker_music.c, src/mp/mp_kaisong.c): a .kaisong copied in as a
# new track (MoemonMaker/music/mus_maker_test.kaisong), made LITTLEROOT TOWN's music, saved ("music MUS_LITTLEROOT
# mus_maker_test"); a game there plays the new track (its own song number, 0x4000, on the BGM player). And a connection
# and a warp added (the game reads them: no edit left out).
# usage: tools/test/maker_music.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_music_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
SONG=$(ls "$ROOT"/music/*.kaisong | head -1)
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_LITTLEROOT_TOWN\nwait 4\n'
    printf 'maker track mus_maker_test %s\nwait 2\nmaker music mus_maker_test\nmaker mode 2\nmaker add warp 9 13\nwait 3\n' "$SONG"
    printf 'makershot map\nmaker save\nmaker close\nwait 60\nquit\n'
} > "$T/edit.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
F="$T/mm/base/MAP_LITTLEROOT_TOWN.txt"
[ -f "$T/mm/music/mus_maker_test.kaisong" ] || { echo "FAIL: the track wasn't copied in"; fail=1; }
grep -q "^music MUS_LITTLEROOT mus_maker_test" "$F" || { echo "FAIL: no music edit"; fail=1; }
grep -q "^add warp 1 dest=MAP_LITTLEROOT_TOWN:0 elevation=[0-9]* x=9 y=13" "$F" || { echo "FAIL: no added warp"; fail=1; }
echo "add connection 9 direction=down map=MAP_ROUTE103 offset=0" >> "$F"
printf 'pressuntil A field 5000\nwait 90\nquit\n' > "$T/play.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" MOEMON_MUSIC_LOG=1 \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 150 "$BIN" > "$T/play.log" 2>&1)
grep -q "kaisong: song 16384 " "$T/play.log" || { echo "FAIL: LITTLEROOT didn't play the new track"; fail=1; }
grep -q "left out\|not understood" "$T/play.log" && { echo "FAIL: edits left out"; grep "left out\|not understood" "$T/play.log"; fail=1; }
[ $fail = 0 ] && echo "maker_music: OK"
exit $fail
