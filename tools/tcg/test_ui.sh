#!/bin/bash
# The card screens with real keys on the normal build: the CARD CLUB (the case, a first
# deck, the teaching duel), a challenge with the registered CARD CASE (win, take a card of
# their deck), the duel's B and CONCEDE, the deck builder and the counter.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/cards-ui}
BIN=${MAIN_BIN:-$REPO/pokeemerald64}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
run() {
    local name=$1 warp=${2:-0,10,8,10}
    rm -rf "${OUT:?}/$name"; mkdir -p "$OUT/$name"
    printf '[player]\nAllowSmallTweaks = 1\nautosave = 0\n' > "$OUT/$name/player.ini"
    printf 'window_scale = 2\n' > "$OUT/$name/display.ini"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_WARP="$warp" \
        MOEMON_TEST="$OUT/$name.txt" MOEMON_SHOTDIR="$OUT/$name" \
        MOEMON_COOP_CONFIG="$OUT/$name/player.ini" MOEMON_DISPLAY_CONFIG="$OUT/$name/display.ini" \
        MOEMON_CONTROLS_CONFIG="$OUT/$name/controls.ini" MOEMON_SAVE="$OUT/$name/player.sav" \
        timeout -k 5 150 "$BIN" > "$OUT/$name/log.txt" 2>&1
}
expect() { grep -q -- "$2" "$OUT/$1/log.txt" || { echo "FAIL $1: no '$2'"; exit 1; }; }

# The CARD CLUB member (OLDALE POKeMON CENTER): the case in dialogue, three decks, the lesson.
cat > "$OUT/club.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
press RIGHT 2
wait 10
pressuntil A cards 3000
wait 100
rendershot reveal
press RIGHT 2
wait 6
press A 2
wait 10
press A 2
wait 40
rendershot got
press A 2
waitfor field 600
pressuntil A cards 2000
wait 60
mp cards fast
mp cards win
wait 200
rendershot lesson-won
press A 2
waitfor field 600
wait 30
pressuntil B field 2000
mp cards state
wait 30
pressuntil A cards 2000
wait 30
rendershot counter
mp cards state
quit
SCRIPT
run club 2,2,11,4
expect club '\[tcg\] starter deck 2'
expect club 'duel started vs CARD CLUB SAM'
expect club 'screen 0 opted 1 .* flags 9 '
expect club 'screen 12 opted 1'
echo 'PASS the CARD CLUB: the case in dialogue, the deck reveal, the teaching duel, then the counter'

# A card player on the map, asked with the registered CARD CASE: win, take one card of their deck.
cat > "$OUT/challenge.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp register 399
wait 10
press UP 2
wait 10
press SELECT 2
wait 10
pressuntil A cards 2000
wait 20
rendershot challenge
press A 2
wait 60
mp cards fast
mp cards win
wait 200
press A 2
wait 30
press A 2
wait 10
press A 2
wait 10
press A 2
wait 30
rendershot took
press A 2
waitfor field 600
mp cards state
press SELECT 2
wait 10
pressuntil A cards 600
wait 20
rendershot again
quit
SCRIPT
run challenge
expect challenge 'asked MANIAC: duel'
expect challenge 'victory vs MANIAC: pack 3'
expect challenge 'took card'
expect challenge 'screen 0 opted 1 .* wins 1 .* packs 0001'
echo 'PASS a challenge with the registered CARD CASE: a win, a booster and one card of their deck'

# The duel: B asks to end the turn (never concedes), CONCEDE asks first.
cat > "$OUT/duel.txt" <<'SCRIPT'
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
press B 2
wait 6
press B 2
wait 6
rendershot endturn
press B 2
wait 6
mp cards state
mp cards focus 10 0
wait 4
press A 2
wait 6
rendershot concede
press B 2
wait 6
mp cards state
quit
SCRIPT
run duel
expect duel 'board asks: End your turn'
expect duel 'board asks: Concede this duel'
test "$(grep -c 'screen 10 opted 1 phase 1' "$OUT/duel/log.txt")" -eq 2
echo 'PASS B asks to end the turn and CONCEDE asks first (no duel ended)'

# The deck builder: take a CHARMANDER out of the deck (the list), add it back (the cards).
cat > "$OUT/builder.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards builder
wait 20
mp cards deck 0
press RIGHT 2
wait 4
press RIGHT 2
wait 4
press RIGHT 2
wait 4
press RIGHT 2
wait 4
press RIGHT 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press A 2
wait 4
mp cards deck 0
rendershot builder
press LEFT 2
wait 4
press A 2
wait 4
mp cards deck 0
quit
SCRIPT
run builder
test "$(grep -c 'deck 0 count 60 valid 1' "$OUT/builder/log.txt")" -eq 2
expect builder 'deck 0 count 59 valid 0'
echo 'PASS the deck builder: out of the deck and back in'

# The counter: spare cards traded in for CARD POINTS, a booster pack bought.
cat > "$OUT/counter.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards own 50 6
mp cards club
wait 20
press A 2
wait 4
press A 2
wait 10
press A 2
wait 4
mp cards points 30
press R 2
wait 4
press A 2
wait 4
press A 2
wait 10
press A 2
wait 4
mp cards state
quit
SCRIPT
run counter
expect counter 'screen 12 opted 1 .* points 18 packs 1000'
echo 'PASS the counter: a spare card traded in, a booster pack bought'
echo "Screenshots and logs: $OUT"
