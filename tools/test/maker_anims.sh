#!/bin/bash
# Moemon Maker M5's move animations (src/maker/maker_anims.c, maker/maker_anims_ui.cpp): MoemonMaker/anims/MOVE_SCRATCH.txt
# replaces SCRATCH's Classic animation (a beam, a flash, a shake, a burst, a sound) in a wild battle; the animation
# starts, is drawn over the battle (pictures in /tmp/maker_anims_test/battle) and ends, and the battle goes on. Then the
# editor's ANIMATIONS window makes one for EMBER, shows frame 20 of its preview and saves it.
# usage: tools/test/maker_anims.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_anims_test
rm -rf "$T"; mkdir -p "$T/battle/mm/anims" "$T/battle/shots" "$T/editor/mm"
fail=0
cat > "$T/battle/mm/anims/MOVE_SCRATCH.txt" <<'S'
anim MOVE_SCRATCH frames=40 replace=1
sound 0 SE_M_SCRATCH
beam 2 16 FFE080 6
flash 16 8 FFFFFF 40
shake 18 12 target 3
burst 18 16 target FFD040 12
ring 18 14 target 80C0FF 4 28
S
cat > "$T/battle/play.txt" <<'S'
pressuntil A field 5000
mp teach 0 10
mp wild 288:5
pressuntil A action 3000
wait 10
press A 2
wait 20
press A 2
pressuntillog A 3000 maker anim MOVE_SCRATCH
wait 18
shot mid
pressuntillog A 600 maker anim done
wait 120
quit
S
(cd "$T/battle" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/battle/play.txt" \
    MOEMON_SHOTDIR="$T/battle/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/battle/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_MAKER_DIR="$T/battle/mm" MOEMON_SAVE="$T/battle/play.sav" timeout 200 "$BIN" > "$T/battle/play.log" 2>&1)
grep "\[maker\]\|maker anim" "$T/battle/play.log" | head -5
want() { grep -q -- "$1" "$2" || { echo "FAIL: no '$1' in $2"; fail=1; }; }
want "\[maker\] 1 move animations" "$T/battle/play.log"
want "maker anim MOVE_SCRATCH (40 frames, in place of the Classic one)" "$T/battle/play.log"
want "maker anim done" "$T/battle/play.log"
grep -q "\[crash\]\|TIMEOUT" "$T/battle/play.log" && { echo "FAIL: crash"; fail=1; }
# the editor
printf 'AllowSmallTweaks = 1\n' > "$T/editor/on.ini"
cat > "$T/editor/play.txt" <<'S'
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
maker anim new MOVE_EMBER
maker anim frame 20
wait 10
makershot anims
maker anim save
wait 5
quit
S
(cd "$T/editor" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/editor/play.txt" \
    MOEMON_SHOTDIR="$T/editor" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/editor/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/editor/mm" timeout 200 "$BIN" > "$T/editor/play.log" 2>&1)
grep -q "^anim MOVE_EMBER frames=48 replace=1" "$T/editor/mm/anims/MOVE_EMBER.txt" 2>/dev/null || { echo "FAIL: the animation wasn't saved"; fail=1; }
want "\[maker\] shot .*anims.bmp ok" "$T/editor/play.log"
[ $fail = 0 ] && echo "maker_anims: OK"
exit $fail
