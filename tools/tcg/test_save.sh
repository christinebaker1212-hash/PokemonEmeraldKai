#!/bin/bash
# Main build: the card collection is saved with the game ("<save>.cards"), comes back
# on Continue, and a new game on the same file starts without cards. Dev builds never
# write it (tools/test/dev_save_test.sh).
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/cards-save}
BIN=${MAIN_BIN:-$REPO/pokeemerald64}
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
rm -rf "$OUT"; mkdir -p "$OUT"
printf '[player]\nAllowSmallTweaks = 0\nautosave = 0\n' > "$OUT/player.ini"
run() {
    local name=$1; shift
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_SKIP_COOP_SETUP=1 "$@" \
        MOEMON_TEST="$OUT/$name.txt" MOEMON_SHOTDIR="$OUT" MOEMON_COOP_CONFIG="$OUT/player.ini" \
        MOEMON_CONTROLS_CONFIG="$OUT/controls.ini" MOEMON_SAVE="${OUT_SAVE:-$OUT/player.sav}" \
        timeout -k 5 90 "$BIN" > "$OUT/$name.log" 2>&1
}
keys() { for k in "$@"; do printf 'press %s 2\nwait 4\n' "$k"; done; }
{
    printf 'pressuntil A field 5000\nwait 300\nmp cards open\n'
    keys A A A
    printf 'mp cards packs\nwait 10\n'
    keys B B
    printf 'wait 30\nmp cards state\nsaveprobe\nwait 60\nquit\n'
} > "$OUT/first.txt"
run first MOEMON_DEBUG_QUICKSTART=1
grep -q 'saveprobe status 1' "$OUT/first.log"
test -s "$OUT/player.sav.cards"
printf 'pressuntil A field 6000\nwait 120\nmp cards state\nmp cards deck 0\nquit\n' > "$OUT/continue.txt"
run continue
grep -q 'screen 0 opted 1 .* wins 1' "$OUT/continue.log"
grep -q 'deck 0 count 60 valid 1' "$OUT/continue.log"
echo 'PASS the collection is saved with the game and back on Continue'
# A new game (another Trainer ID) next to that collection file.
printf 'pressuntil A field 5000\nwait 120\nmp cards state\nquit\n' > "$OUT/newgame.txt"
mkdir -p "$OUT/new"
cp "$OUT/player.sav.cards" "$OUT/new/player.sav.cards"
OUT_SAVE="$OUT/new/player.sav" run newgame MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_TID=12345
if grep -q 'screen 0 opted 1' "$OUT/newgame.log"; then
    echo 'FAIL a new game inherited the previous collection'; exit 1
fi
echo 'PASS a new game with another Trainer ID starts without cards'
# Control: the same file next to a game with the collection's own Trainer ID keeps it.
mkdir -p "$OUT/same"
cp "$OUT/player.sav.cards" "$OUT/same/player.sav.cards"
cp "$OUT/newgame.txt" "$OUT/samegame.txt"
OUT_SAVE="$OUT/same/player.sav" run samegame MOEMON_DEBUG_QUICKSTART=1
grep -q 'screen 0 opted 1' "$OUT/samegame.log"
echo 'PASS the same Trainer ID keeps its collection'
