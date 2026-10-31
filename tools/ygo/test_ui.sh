#!/bin/bash
# The Yu-Gi-Oh! screens with real keys on the normal build: the CARD CLUB's duelist (the CARD CASE,
# a Starter Deck, the teaching duel, the counter), a challenge with the registered CARD CASE
# (win, a booster, one card of their deck), the case's screens, and a practice duel mashed with
# random keys until it ends.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/ygo-ui}
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

# The CARD CLUB's Yu-Gi-Oh! duelist (OLDALE POKeMON MART): the CARD CASE, a Starter Deck,
# the teaching duel, then the counter the next time.
if only club; then
cat > "$OUT/club.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
press UP 2
wait 10
pressuntil A cards 3000
wait 60
rendershot starters
press RIGHT 2
wait 6
press A 2
wait 10
rendershot ask
press A 2
wait 10
press A 2
wait 10
mp ygo state
pressuntil A cards 2000
wait 60
mp ygo fast
rendershot lesson
mp ygo win
wait 320
rendershot lesson-won
press A 2
waitfor field 1200
wait 30
pressuntil B field 2000
mp ygo state
wait 30
pressuntil A cards 2000
wait 30
rendershot counter
mp ygo state
quit
SCRIPT
run club 2,4,7,3
expect club '\[ygo\] screen [0-9]* opted 1 starter [1-9][0-9]* deck [45][0-9]+0'
expect club 'duel started vs CARD CLUB DUELIST'
expect club 'left the duel (won)'
expect club 'screen 12 opted 1'
echo 'PASS the CARD CLUB duelist: the case, a Starter Deck, the teaching duel, then the counter'
fi

# The CARD CLUB duelist asked with the registered CARD CASE: "MOeMON CARDS or YU-GI-OH!", a win,
# a booster and one card of their deck.
if only challenge; then
cat > "$OUT/challenge.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp ygo starter 0
mp register 399
wait 10
press UP 2
wait 10
press SELECT 2
wait 400
press A 2
wait 30
rendershot which
press DOWN 2
wait 6
press A 2
wait 10
pressuntil A cards 600
wait 20
rendershot challenge
press A 2
wait 60
mp ygo fast
mp ygo win
wait 320
rendershot won
press A 2
wait 30
press A 2
wait 10
rendershot spoils
press INSPECT 2
wait 20
rendershot spoils_look
press B 2
wait 20
press A 2
wait 10
press A 2
wait 30
rendershot took
press A 2
waitfor field 600
mp ygo state
quit
SCRIPT
run challenge 2,4,7,3
expect challenge '\[ygo\] challenge: CARD CLUB'
expect challenge 'victory vs CARD CLUB'
expect challenge 'inspect card [0-9]* from screen'   # R2 on the card to take
expect challenge 'screen 0 opted 1 .* packs 1 wins 1 '
echo 'PASS a challenge with the registered CARD CASE: which game, a win, a booster and one card of their deck'
fi

# The case's screens, drawn (look at the rendershots).
if only screens; then
cat > "$OUT/screens.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 1
mp ygo case
wait 30
rendershot home
mp ygo binder
wait 30
rendershot binder
press R 2
wait 10
press DOWN 2
wait 10
rendershot binder-normal
mp ygo builder
wait 30
rendershot builder
press A 2
wait 10
rendershot builder-a
press B 2
wait 10
mp ygo packs 0 3
mp ygo pack 0
wait 30
rendershot pack
wait 150
rendershot revealing
wait 250
rendershot reveal
mp ygo inspect 1
wait 30
rendershot inspect
press A 2
wait 20
rendershot inspect-big
press B 2
wait 10
press B 2
wait 10
mp ygo state
quit
SCRIPT
run screens
expect screens 'opened LOB'
refuse screens 'heap\] out of memory'
echo 'PASS the case: home, binder, builder, a booster opened, a card inspected'
fi

# Deck codes: FROM CODE with a YDKE code (owned cards and missing ones, shown gray), a code with a card the game hasn't
# got (refused), and the Forbidden & Limited list changed on the DECKS screen
if only codes; then
cat > "$OUT/codes.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 1
mp ygo code ydke://ryPeAIDfbgU=!!!
wait 20
rendershot code-bad
press A 2
wait 20
rendershot code-refused
press A 2
wait 10
mp ygo code ydke://rvTMAoDfbgWA324F!!!
wait 20
rendershot code
press A 2
wait 30
rendershot code-made
press A 2
wait 10
press UP 2
wait 6
press UP 2
wait 6
press UP 2
wait 6
press UP 2
wait 6
press RIGHT 2
wait 10
rendershot list
quit
SCRIPT
run codes
expect codes 'cards not in the game'
expect codes 'deck CODE DECK into slot 1: 3 cards'
expect codes 'list: 2026.10 TCG'
echo 'PASS deck codes: a YDKE code made into a deck (cards not owned yet included), one with a card the game lacks refused, the list changed'
fi

# A practice duel mashed with random keys for a while, then played out by the AI: the cards are
# all still there, the duel ends, no flood of refused answers, no crash.
if only monkey; then
{
    echo 'pressuntil A field 5000'
    echo 'wait 60'
    echo 'mp ygo starter 2'
    echo 'mp ygo first 0'
    echo 'mp ygo fast'
    echo 'mp ygo practice'
    echo 'wait 120'
    python3 - <<'PY'
import random
r = random.Random(int(__import__('os').environ.get('SEED', '7')))
keys = ['A'] * 8 + ['B'] * 3 + ['START', 'R', 'L', 'SELECT'] + ['UP', 'DOWN', 'LEFT', 'RIGHT'] * 3
for i in range(900):
    print('press %s 2' % r.choice(keys))
    print('wait %d' % r.choice([2, 4, 6, 12]))
    if i % 150 == 0:
        print('mp ygo state')
        print('rendershot m%03d' % (i // 150))
PY
    echo 'mp ygo state'
    # back out of a card's picture or details to the table
    for i in 1 2 3; do echo 'press B 2'; echo 'wait 12'; done
    echo 'mp ygo auto'
    for i in 1 2 3 4 5 6 7 8; do echo 'wait 2000'; echo 'mp ygo state'; done
    echo 'rendershot end'
    echo 'mp ygo state'
    echo 'quit'
} > "$OUT/monkey.txt"
TIMEOUT=600 run monkey
expect monkey 'duel started vs PRACTICE DECK'
refuse monkey 'conserved 0'
refuse monkey 'empty chain'
expect monkey 'over 1'
n=$(grep -c 'answer refused' "$OUT/monkey/log.txt" || true)
[ "$n" -lt 20 ] || { echo "FAIL monkey: $n refused answers"; exit 1; }
echo "PASS a practice duel mashed with random keys, then played out ($n refused answers)"
fi

# The GAME CENTER's Yu-Gi-Oh! clerk (RUSTBORO CITY 2F): a CARD CASE and a Starter Deck for a new duelist, then a
# single and a Starter Deck for money (one each: the second time they're SOLD).
if only gcshop; then
cat > "$OUT/gcshop.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
press LEFT 2
wait 10
pressuntil A cards 4000
wait 30
press A 2
wait 10
press A 2
wait 10
press A 2
wait 10
pressuntil A cards 4000
wait 40
rendershot singles
press A 2
wait 20
press A 2
wait 10
rendershot buy
press A 2
wait 10
press A 2
wait 10
press A 2
wait 10
rendershot sold
press B 2
wait 10
press B 2
wait 10
press R 2
wait 10
rendershot decks
press DOWN 2
wait 4
press A 2
wait 10
press A 2
wait 60
rendershot bought-deck
press A 2
wait 10
press A 2
wait 10
rendershot deck-sold
press A 2
wait 10
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
press DOWN 2
wait 4
rendershot structure
press A 2
wait 10
press A 2
wait 60
rendershot bought-structure
press A 2
wait 10
mp ygo state
mp ygo recipes
wait 20
rendershot recipes
quit
SCRIPT
MOEMON_DEBUG_MONEY=50000 run gcshop 11,18,2,6
expect gcshop 'game center: bought card'
expect gcshop 'game center: bought deck 1'
refuse gcshop 'game center: bought deck 1, .*game center: bought deck 1'
expect gcshop 'game center: bought deck 7'   # (Structure Deck: Zombie Madness, further down the list)
refuse gcshop 'recipe [0-9]* .* valid 0'
expect gcshop 'recipe [0-9]* Zombie Madness:'   # (a deck bought is a recipe)
refuse gcshop "recipe [0-9]* Dragon's Roar:"   # (one not bought isn't)
expect gcshop 'recipe [0-9]* WALLY:'   # (and every special duelist beaten)
echo 'PASS the GAME CENTER Yu-Gi-Oh! clerk: a new duelist set up, a single, a Starter and a Structure Deck for money, one each; DECK RECIPES of decks bought and duelists beaten, all legal'

fi

# Asked for a Yu-Gi-Oh! duel with only the POKEMON cards: a Starter Deck to keep first, then the duel. With
# Yu-Gi-Oh! cards but no deck ready: a Starter Deck to borrow for this duel.
if only resolve; then
for mode in keep loan; do
{
    echo 'pressuntil A field 5000'
    echo 'wait 100'
    echo 'mp cards starter 0'
    [ $mode = loan ] && { echo 'mp ygo starter 2'; echo 'mp ygo nodecks'; }
    cat <<'SCRIPT'
mp ygo first 1
mp register 399
wait 10
press UP 2
wait 10
press SELECT 2
wait 400
press A 2
wait 30
press DOWN 2
wait 6
press A 2
wait 10
pressuntil A cards 600
wait 20
rendershot resolve
press A 2
wait 10
press RIGHT 2
wait 6
press A 2
wait 10
rendershot ask
press A 2
wait 20
rendershot after
press A 2
wait 20
press A 2
wait 60
mp ygo state
mp ygo fast
mp ygo win
wait 320
press A 2
wait 30
press A 2
wait 10
press A 2
wait 10
press A 2
wait 30
press A 2
waitfor field 600
mp ygo state
quit
SCRIPT
} > "$OUT/resolve_$mode.txt"
run resolve_$mode 2,4,7,3
done
expect resolve_keep 'no deck: a Starter Deck to keep'
expect resolve_keep 'no deck: took the SD'
expect resolve_keep 'duel started vs CARD CLUB'
expect resolve_keep 'victory vs CARD CLUB'
expect resolve_keep 'opted 1 starter [0-9]* .* wins 1 '
expect resolve_loan 'no deck: a Starter Deck to borrow'
expect resolve_loan 'no deck: borrowed the SD'
expect resolve_loan 'duel started vs CARD CLUB'
expect resolve_loan 'victory vs CARD CLUB'
expect resolve_loan 'deck 0+0 .* wins 1 '
echo 'PASS no Yu-Gi-Oh! deck: a Starter Deck to keep (POKEMON cards only) or to borrow (no deck ready), then the duel'
fi

# A held as long as a person holds it (10 frames) to close the message that opens a Yu-Gi-Oh! screen: the screen
# opens and that A does nothing in it (it used to pick the first row: the clerk's SINGLES, a Starter Deck).
if only held; then
cat > "$OUT/held_counter.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 0
press LEFT 2
wait 10
press A 2
wait 300
press A 10
wait 20
mp ygo state
rendershot counter
quit
SCRIPT
run held_counter 11,18,2,6
expect held_counter '\[ygo\] screen 12 opted 1 .* message 0'
cat > "$OUT/held_starter.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp textspeed 2
mp additem 399 1
press UP 2
wait 10
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
rendershot lastpage
press A 10
wait 20
mp ygo state
rendershot starter
quit
SCRIPT
run held_starter 2,4,7,3
expect held_starter '\[ygo\] screen 13 opted 0 .* message 0'
echo 'PASS an A held to close a message opens the Yu-Gi-Oh! screen without choosing anything in it'
fi

# The table: from the Battle Phase, MAIN PHASE 2 (chosen) or END TURN, which skips it; a Field Spell's picture as the
# background (Yami, from its artwork); the opponent's main Attribute as the table's theme
if only zone; then
# The engine's zone question for a Summon lights the Monster Zones (the Decks unshuffled: Summoned Skull in the opening
# hand; the AI plays your turns until the engine asks where it goes)
cat > "$OUT/zone.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 1
mp ygo giveall 3
mp ygo code ydke://9slUAvbJVAL2yVQCo6lXBaOpVwWjqVcFdHyKBHR8igR0fIoEOnjsATp47AE6eOwBtPTMArT0zAK09MwCJithACYrYQAmK2EA+21PBfttTwX7bU8FixJoAosSaAKLEmgCYr1iBWK9YgVivWIFAsWLAALFiwACxYsAwY2jBMGNowTBjaMEIP5rBSD+awWA324FfAg4BNIw7wH7mncCtEblAA==!!!
wait 20
press A 2
wait 30
press A 2
wait 10
mp ygo stack
mp ygo fast
mp ygo seed 7
mp ygo first 0
mp ygo open
wait 20
press DOWN 2
wait 6
press A 2
wait 300
mp ygo mineuntil 8
waitlog 9000 ygo mine reached
wait 40
rendershot tribute_zone
mp ygo state
quit
SCRIPT
run zone
expect zone 'ygo mine reached 8'
echo 'PASS the engine asks where a Summoned monster goes: the Monster Zones light up (screenshot tribute_zone)'
fi

# Counters: Breaker the Magical Warrior Normal Summoned gets its Spell Counter, a violet badge on the table and a line
# on its close-up (screenshots counter_table, counter_inspect)
if only counters; then
cat > "$OUT/counters.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 0
mp ygo decktop 918 912 120
mp ygo stack
mp ygo seed 7
mp ygo first 0
mp ygo practice
wait 200
mp ygo mineuntil 1
waitlog 6000 ygo mine reached
wait 30
mp ygo mine
wait 150
mp ygo mine
wait 150
mp ygo mine
waitlog 600 got a Spell Counter
wait 60
mp ygo focus 1 0
wait 10
mp ygo focus 1 1
wait 10
mp ygo focus 1 2
wait 10
rendershot counter_table
press INSPECT 2
wait 40
rendershot counter_inspect
quit
SCRIPT
run counters
expect counters 'Breaker the Magical Warrior got a Spell Counter (1 now)'
expect counters 'inspect spot [0-9]* obj [0-9]* counters 1'
echo 'PASS counters: a Spell Counter on the table (badge) and on the close-up (screenshots counter_table, counter_inspect)'
fi

if only table; then
cat > "$OUT/table.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp ygo starter 0
mp ygo fast
mp ygo seed 7
mp ygo first 0
mp ygo open
wait 20
press A 2
wait 300
mp ygo mineuntil 2
waitlog 6000 ygo mine reached
wait 30
press B 2
wait 20
rendershot battle_menu
press DOWN 2
wait 6
press A 2
wait 60
mp ygo mineuntil 1
waitlog 6000 ygo mine reached
mp ygo state
mp ygo fieldbg 52
wait 20
press RIGHT 2
wait 20
rendershot field_bg
quit
SCRIPT
run table
expect table 'turn menu: cursor on MAIN PHASE 2'
expect table 'ygo board: chose END TURN'
expect table 'duel: turn [0-9]* phase 2 current 0'
refuse table 'phase 4'
expect table 'field background: Yami'
echo 'PASS the Battle Phase menu (MAIN PHASE 2 chosen, END TURN skips it) and a Field Spell background'
fi

# HOW TO PLAY: SELECT on the case's first screen, every page, B: back on the same screen (the case still open)
if only guide; then
cat > "$OUT/guide.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp ygo starter 0
mp ygo case
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
mp ygo state
rendershot guide_closed
quit
SCRIPT
run guide
expect guide 'HOW TO PLAY opened (game 1, 10 pages)'
expect guide 'page 10: CONTROLS'
expect guide 'guide\] closed'
expect guide 'ygo\] screen 1 '
refuse guide "words don't fit"
echo 'PASS HOW TO PLAY: every page, then back on the case'
fi
