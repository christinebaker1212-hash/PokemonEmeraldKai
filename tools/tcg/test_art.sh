#!/bin/bash
# OPTIONS > DISPLAY > CARD ART on the normal build: the same duel in each mode (PRINTED ART,
# GAME BOY, FULL CARD), its board, a card up close and the big picture, and CARD BACKS: PIXEL.
# The modes must draw different cards; the pictures are in dev/test-results/cards-art for a look.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/cards-art}
BIN=${MAIN_BIN:-$REPO/pokeemerald64}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
cat > "$OUT/art.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards fast
mp cards open
wait 20
press A 2
wait 200
mp cards ready
wait 300
rendershot board
mp cards focus 0 0
wait 4
press START 2
wait 30
rendershot inspect
press A 2
wait 30
rendershot big
press B 2
wait 10
press B 2
wait 10
mp cards focus 0 99
wait 4
press START 2
wait 30
rendershot inspect2
press A 2
wait 30
rendershot big2
quit
SCRIPT
for run in printed_art gameboy full_card pixel_back; do
    mode=$run back=printed
    if [ "$run" = pixel_back ]; then mode=printed_art back=pixel; fi
    rm -rf "${OUT:?}/$run"; mkdir -p "$OUT/$run"
    printf '[player]\nAllowSmallTweaks = 1\nautosave = 0\ncard_art = %s\ncard_back = %s\n' "$mode" "$back" > "$OUT/$run/player.ini"
    mode=$run
    printf 'window_scale = 2\n' > "$OUT/$mode/display.ini"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_WARP=0,10,8,10 \
        MOEMON_TEST="$OUT/art.txt" MOEMON_SHOTDIR="$OUT/$mode" \
        MOEMON_COOP_CONFIG="$OUT/$mode/player.ini" MOEMON_DISPLAY_CONFIG="$OUT/$mode/display.ini" \
        MOEMON_CONTROLS_CONFIG="$OUT/$mode/controls.ini" MOEMON_SAVE="$OUT/$mode/player.sav" \
        timeout -k 5 150 "$BIN" > "$OUT/$mode/log.txt" 2>&1
    grep -q "card_back = $back" "$OUT/$mode/player.ini" || { echo "FAIL $mode: the setting didn't stay"; exit 1; }
    for shot in board inspect big; do
        [ -s "$OUT/$mode/$shot.bmp" ] || { echo "FAIL $mode: no $shot picture"; exit 1; }
    done
done
for shot in board inspect big; do
    a=$(md5sum < "$OUT/printed_art/$shot.bmp") b=$(md5sum < "$OUT/gameboy/$shot.bmp") c=$(md5sum < "$OUT/full_card/$shot.bmp")
    if [ "$a" = "$b" ] || [ "$a" = "$c" ] || { [ "$shot" != big ] && [ "$b" = "$c" ]; }; then
        echo "FAIL the $shot picture is the same in two CARD ART modes"; exit 1
    fi
done
[ "$(md5sum < "$OUT/printed_art/board.bmp")" != "$(md5sum < "$OUT/pixel_back/board.bmp")" ] ||
    { echo "FAIL CARD BACKS: PRINTED and PIXEL look the same"; exit 1; }
echo "PASS CARD ART: PRINTED ART, GAME BOY and FULL CARD draw different cards; CARD BACKS too (pictures in $OUT)"
