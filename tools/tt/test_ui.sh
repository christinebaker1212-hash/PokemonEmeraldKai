#!/bin/bash
# The Triple Triad screens with real keys on the normal build: a seller (the CARD CASE, the first hand, the counter),
# the case's screens, a challenge with the registered CARD CASE (rules on the move, a game, the cards taken), the
# CARD QUEEN, and practice games played to the end.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/tt-ui}
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
        timeout -k 5 ${TIMEOUT:-150} "$BIN" > "$OUT/$name/log.txt" 2>&1
}
expect() { grep -q -- "$2" "$OUT/$1/log.txt" || { echo "FAIL $1: no '$2'"; exit 1; }; }
refuse() { if grep -q -- "$2" "$OUT/$1/log.txt"; then echo "FAIL $1: '$2'"; exit 1; fi; }
only() { [ -z "${ONLY:-}" ] || [[ " $ONLY " == *" $1 "* ]]; }

# PETALBURG's seller (its POKeMON CENTER): the CARD CASE, the first hand, the counter
if only seller; then
cat > "$OUT/seller.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
wait 60
mp face right
wait 10
rendershot pc
pressuntil A cards 3000
wait 40
rendershot starter-deal
wait 120
rendershot starter
press A 2
wait 20
rendershot starter-done
mp tt state
mp money 50000
press A 2
wait 40
pressuntil A cards 8000
wait 30
rendershot shop
press DOWN 2
wait 4
press A 2
wait 10
rendershot buy-ask
press A 2
wait 10
rendershot bought
press R 2
wait 10
press DOWN 2
wait 4
press DOWN 2
wait 4
press A 2
wait 10
press A 2
wait 10
rendershot pack-ask
press A 2
wait 30
rendershot pack-open
wait 80
rendershot pack-done
press A 2
wait 10
press DOWN 2
wait 4
press DOWN 2
wait 4
rendershot not-sold
press A 2
wait 10
rendershot not-sold-msg
press A 2
wait 10
mp tt give 2 3
press R 2
wait 10
rendershot sell
press A 2
wait 10
press A 2
wait 10
rendershot sold
mp tt state
quit
SCRIPT
run seller 8,4,12,4
expect seller '\[tt\] screen [0-9]* cards 5 decks ready 1 region PETALBURG'
expect seller '\[tt\] screen 12 cards 9 decks'
echo 'PASS the seller: the case, the first hand, the counter'
fi

# The case's screens
if only case; then
cat > "$OUT/case.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
wait 30
mp tt starter
mp tt give 100
mp tt give 200
mp tt give 250
mp tt give 300 3
mp tt packs 2 2
mp tt packs 4 1
mp tt case
wait 30
rendershot home
press DOWN 2
wait 6
press A 2
wait 20
rendershot list
press A 2
wait 20
rendershot inspect
press B 2
wait 10
press R 2
wait 10
press R 2
wait 10
rendershot list3
press B 2
wait 10
press UP 2
wait 6
press A 2
wait 10
rendershot decks
press DOWN 2
wait 4
press A 2
wait 6
rendershot deckmenu
press A 2
wait 10
rendershot builder
press A 2
wait 6
press RIGHT 2
wait 4
press A 2
wait 6
press RIGHT 2
wait 4
press A 2
wait 6
press RIGHT 2
wait 4
press A 2
wait 6
press R 2
wait 6
press R 2
wait 6
rendershot builder2
press A 2
wait 6
rendershot builder3
press START 2
wait 10
press A 2
wait 20
rendershot decks2
press B 2
wait 10
press DOWN 2
wait 4
press DOWN 2
wait 4
press A 2
wait 10
rendershot packs
press DOWN 2
wait 4
press DOWN 2
wait 4
press A 2
wait 30
rendershot reveal1
wait 60
rendershot reveal2
press A 2
wait 10
press B 2
wait 10
press DOWN 2
wait 4
press A 2
wait 10
rendershot rules
press DOWN 2
wait 4
rendershot rules2
press B 2
wait 10
mp tt state
press L 2
wait 10
rendershot ygo-side
press L 2
wait 10
rendershot pokemon-side
press L 2
wait 10
rendershot tt-side
mp tt state
quit
SCRIPT
run case 8,4,12,4
expect case 'screen 1 cards 1[0-9] decks ready 1'
echo 'PASS the case screens'
fi

# A practice game played to the end by the computer on both sides
if only practice; then
cat > "$OUT/practice.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
wait 30
mp tt starter
mp tt first 0
mp tt rules 0 4097
mp tt case
wait 10
mp tt practice
wait 8
rendershot deal
wait 50
rendershot rules
press A 2
wait 40
rendershot spin
wait 90
rendershot play
mp tt focus 7
wait 10
rendershot cursor
press A 2
wait 2
rendershotburst place 30 3
wait 40
rendershot placed
mp tt auto
wait 300
rendershot midgame
waitlog 4000 game over
wait 100
rendershot end
press A 2
waitlog 600 left the table
wait 30
rendershot after
quit
SCRIPT
run practice 8,4,12,4
expect practice '\[tt\] game vs PRACTICE'
expect practice 'left the table'
echo 'PASS a practice game'
fi

# The GAME CENTER 2F: the TRIPLE TRIAD seller's counter, a player beside it asked for a game (rules on the move: the
# PETALBURG rules carried in), a win, a card of theirs taken
if only gamecenter; then
cat > "$OUT/gamecenter.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp tt starter
mp tt carry 0
mp tt rules 0 1
mp tt rules 1 2
mp tt queen 1
mp tt first 0
mp face right
wait 10
rendershot room
pressuntil A cards 3000
wait 30
rendershot challenge
pressuntil A cards 600
mp tt state
press A 2
wait 20
rendershot challenge2
press A 2
wait 30
press A 2
wait 200
rendershot table
mp tt win
waitlog 600 game over
wait 120
rendershot won
press A 2
wait 30
rendershot spoils
press RIGHT 2
wait 6
press A 2
wait 10
rendershot take
press A 2
wait 20
press A 2
wait 30
mp tt state
quit
SCRIPT
run gamecenter 11,18,11,8
expect gamecenter 'challenge: BARD in RUSTBORO'
expect gamecenter 'left the table (won'
expect gamecenter 'wins 1'
echo 'PASS the GAME CENTER: a game with the player by the TRIPLE TRIAD seller, a card taken'
fi

# A SCHOOL KID by the GAME CENTER's seller plays MOeMON cards and TRIPLE TRIAD: they ask which
if only which; then
cat > "$OUT/which.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp tt starter
mp textspeed 2
mp face right
wait 10
press A 2
wait 90
press A 2
wait 30
rendershot which
press DOWN 2
wait 6
press A 2
wait 40
rendershot chose
mp tt state
quit
SCRIPT
run which 11,18,11,9
expect which 'asked SCHOOL KID: which game'
expect which 'challenge: SCHOOL KID in RUSTBORO'
echo 'PASS a player of two games asks which'
fi

# The CARD QUEEN in MAUVILLE's POKeMON CENTER: her menu, a journey paid for, gone from the room
if only queen; then
cat > "$OUT/queen.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp tt starter
mp textspeed 2
mp face left
wait 10
rendershot room
pressuntil A cards 3000
wait 20
rendershot menu
mp tt favor
press DOWN 2
wait 4
press DOWN 2
wait 4
rendershot favor
press A 2
wait 10
rendershot trade
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press A 2
wait 10
press A 2
wait 10
press A 2
wait 10
mp tt state
press UP 2
wait 4
press A 2
wait 10
press DOWN 2
wait 4
press A 2
wait 10
rendershot travel
press A 2
wait 10
press A 2
wait 20
mp tt state
pressuntil A field 2000
wait 60
rendershot gone
mp tt state
quit
SCRIPT
run queen 10,5,1,5
expect queen 'region MAUVILLE rules [0-9a-f]* trade 3'
expect queen 'queen LAVARIDGE'
echo 'PASS the CARD QUEEN: a favor (the TRADE RULE ALL), a journey'
fi

# HOW TO PLAY: SELECT on the case's first screen, every page, B: back on the same screen (the case still open)
if only guide; then
cat > "$OUT/guide.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp tt starter
mp tt case
wait 30
press SELECT 2
wait 40
rendershot guide_first
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
press RIGHT 2
wait 12
wait 60
rendershot guide_last
press LEFT 2
wait 10
press B 4
wait 30
mp tt state
rendershot guide_closed
quit
SCRIPT
run guide
expect guide 'HOW TO PLAY opened (game 2, 10 pages)'
expect guide 'page 10: CONTROLS'
expect guide 'guide\] closed'
expect guide 'tt\] screen 1 '
refuse guide "words don't fit"
echo 'PASS HOW TO PLAY: every page, then back on the case'
fi
