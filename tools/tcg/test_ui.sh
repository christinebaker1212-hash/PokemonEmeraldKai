#!/bin/bash
# Exercise actual controller input against the native dev game, not a UI mock.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/cards-ui}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then ./dev/build.sh linux 1; fi
BIN=${DEV_BIN:-$REPO/dev/builds/linux/MoemonEmeraldKaiCoop}
run() {
    local name=$1 warp=${2:-0,10,8,10}
    mkdir -p "$OUT/$name"
    printf '[player]\nAllowSmallTweaks = 1\nautosave = 1\n' > "$OUT/$name/player.ini"
    printf 'window_scale = 4\n' > "$OUT/$name/display.ini"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_WARP="$warp" \
        MOEMON_TEST="$OUT/$name.txt" MOEMON_SHOTDIR="$OUT/$name" \
        MOEMON_COOP_CONFIG="$OUT/$name/player.ini" MOEMON_DISPLAY_CONFIG="$OUT/$name/display.ini" \
        MOEMON_CONTROLS_CONFIG="$OUT/$name/controls.ini" MOEMON_SAVE="$OUT/$name/player.sav" \
        MOEMON_TCG_ART_DIR="$REPO/dev/artwork/tcg-art" \
        timeout -k 5 60 "$BIN" > "$OUT/$name/log.txt" 2>&1
    test -z "$(find "$OUT/$name" -name 'player.sav*')"
}
cat > "$OUT/editor.txt" <<'EOF'
pressuntil A field 5000
wait 600
mp cards open
press B 2
wait 5
mp cards state
mp cards open
press A 2
wait 5
press A 2
wait 5
press DOWN 2
wait 3
press DOWN 2
wait 3
press A 2
wait 3
press A 2
wait 3
press DOWN 2
wait 3
press A 2
wait 3
# Grass category may be empty; move to Fire, which the chosen deck includes.
press R 2
wait 3
rendershot deck-builder
mp cards deck 0
# Remove one Fire Pokemon, then add it back with the original GB controls.
press LEFT 2
wait 3
mp cards deck 0
press RIGHT 2
wait 3
mp cards deck 0
press START 2
wait 3
press A 2
wait 3
# Return to the deck's action menu; copy to the second empty slot.
press B 2
wait 3
press DOWN 2
wait 3
press DOWN 2
wait 3
press A 2
wait 3
press DOWN 2
wait 3
press A 2
wait 3
mp cards deck 1
rendershot deck-copy-result
mp cards state
quit
EOF
run editor
grep -q 'screen 0 opted 0' "$OUT/editor/log.txt"
grep -q 'deck 0 count 59 valid 0' "$OUT/editor/log.txt"
test "$(grep -c 'deck 0 count 60 valid 1' "$OUT/editor/log.txt")" -eq 2
grep -q 'deck 1 count 60 valid 1' "$OUT/editor/log.txt"
echo 'PASS decline, explicit opt-in, GB deck controls, legality and independent deck copy'
cat > "$OUT/field.txt" <<'EOF'
pressuntil A field 5000
wait 600
press UP 2
wait 30
mp cards challenge
wait 5
rendershot field-challenge
mp cards state
press B 2
wait 5
mp cards state
quit
EOF
run field
grep -q 'field challenge accepted: 1' "$OUT/field/log.txt"
grep -q 'screen 1 opted 0' "$OUT/field/log.txt"
grep -q 'screen 0 opted 0' "$OUT/field/log.txt"
echo 'PASS real Oldale NPC challenge, decline and no save/profile writes'
echo "Screenshots and logs: $OUT"
