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
        timeout -k 5 90 "$BIN" > "$OUT/$name/log.txt" 2>&1
    test -z "$(find "$OUT/$name" -name 'player.sav*')"
}
# Each key press is released before the next one (menus react to new presses).
keys() { for k in "$@"; do printf 'press %s 2\nwait 4\n' "$k"; done; }

{
    printf 'pressuntil A field 5000\nwait 600\nmp cards open\n'
    keys B
    printf 'mp cards state\nmp cards open\n'
    # YES, the first starter deck, dismiss the note, DECKS, deck 1, EDIT CARDS.
    keys A A A DOWN DOWN A A DOWN A
    printf 'rendershot deck-builder\nmp cards deck 0\n'
    # Remove one Fire Pokemon, then add it back with the original GB controls.
    keys LEFT
    printf 'mp cards deck 0\n'
    keys RIGHT
    printf 'mp cards deck 0\n'
    keys START A
    # Back to the deck's actions; COPY DECK into the second (empty) slot.
    keys B DOWN A DOWN A
    printf 'mp cards deck 1\nrendershot deck-copy-result\nmp cards state\nquit\n'
} > "$OUT/editor.txt"
run editor
grep -q 'screen 0 opted 0' "$OUT/editor/log.txt"
grep -q 'deck 0 count 59 valid 0' "$OUT/editor/log.txt"
test "$(grep -c 'deck 0 count 60 valid 1' "$OUT/editor/log.txt")" -eq 2
grep -q 'deck 1 count 60 valid 1' "$OUT/editor/log.txt"
echo 'PASS decline, explicit opt-in, GB deck controls, legality and independent deck copy'

cat > "$OUT/field.txt" <<'SCRIPT'
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
SCRIPT
run field
grep -q 'field challenge accepted: 1' "$OUT/field/log.txt"
grep -q 'screen 1 opted 0' "$OUT/field/log.txt"
grep -q 'screen 0 opted 0' "$OUT/field/log.txt"
echo 'PASS real Oldale NPC challenge, decline and no save/profile writes'

# The duel board: setup with real keys, then checking cards changes nothing.
{
    printf 'pressuntil A field 5000\nwait 600\nmp cards starter 0\nmp cards fast\nmp cards open\n'
    keys A
    # After the coin and the deal the cursor waits on a Basic Pokemon:
    # A (menu), A (PLAY), A (the Active space), then READY and YES.
    printf 'wait 200\nrendershot duel-setup\n'
    keys A A
    printf 'rendershot duel-target\n'
    keys A
    printf 'wait 30\nmp cards state\nmp cards focus 10 0\nwait 4\n'
    keys A
    printf 'rendershot duel-ready\n'
    keys A
    printf 'wait 30\nmp cards ready\nwait 400\nmp cards state\nmp cards snapshot\nrendershot duel-main\n'
    # Look at the opponent's Active card, their discard pile and the quick menu.
    printf 'mp cards focus 2 0\nwait 4\n'
    keys A
    printf 'mp cards state\nrendershot duel-inspect\n'
    keys B
    printf 'mp cards focus 4 0\nwait 4\n'
    keys A
    printf 'rendershot duel-discard\n'
    keys B SELECT
    printf 'rendershot duel-quick\n'
    keys B
    printf 'mp cards state\nmp cards snapshot\n'
    # Attach an Energy card from the hand to the Active Pokemon with keys.
    printf 'mp cards give 2\nwait 10\nmp cards focus 0 99\nwait 30\n'
    keys A A
    printf 'rendershot duel-attach-target\n'
    keys A
    printf 'wait 60\nmp cards state\nrendershot duel-attached\nquit\n'
} > "$OUT/duel.txt"
run duel
grep -q 'screen 10 opted 1 phase 0 current 0 wins 0 hand 6 field 1' "$OUT/duel/log.txt"
grep -q 'ready phase 1 current 0 conserved 1' "$OUT/duel/log.txt"
grep -q 'screen 5 opted 1 phase 1' "$OUT/duel/log.txt"
test "$(grep -c 'rules snapshot' "$OUT/duel/log.txt")" -eq 2
test "$(grep 'rules snapshot' "$OUT/duel/log.txt" | sort -u | wc -l)" -eq 1
grep -q 'give 2 -> 0 conserved 1' "$OUT/duel/log.txt"
grep 'screen 10 opted 1 phase 1' "$OUT/duel/log.txt" | tail -1 | grep -q 'energy [1-9]'
echo 'PASS board setup with keys, card checks leave the rules state unchanged, Energy attached with keys'
echo "Screenshots and logs: $OUT"
