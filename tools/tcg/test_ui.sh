#!/bin/bash
# The card screens with real keys on the normal build: the CARD CLUB (the case, a first
# deck, the teaching duel), a challenge with the registered CARD CASE (win, take a card of
# their deck), the duel's B and CONCEDE on the deck, the deck builder and the counter.
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
refuse() { if grep -q -- "$2" "$OUT/$1/log.txt"; then echo "FAIL $1: '$2'"; exit 1; fi; }

# The CARD CLUB member (OLDALE POKeMON MART, behind the counter): the case in dialogue, three decks, the lesson.
cat > "$OUT/club.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
press LEFT 2
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
wait 320
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
run club 2,4,3,2
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
wait 320
press A 2
wait 30
press A 2
wait 10
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
expect challenge 'challenge picture: graphics [0-9]*, trainer picture [0-9]'   # their battle picture, cut to the window
expect challenge 'victory vs MANIAC: pack [0-9]*'
expect challenge 'inspect card [0-9]* from screen'   # R2 on the card to take
expect challenge 'took card'
expect challenge 'screen 0 opted 1 .* wins 1 .* packs 0*10* '
echo 'PASS a challenge with the registered CARD CASE: a win, a booster and one card of their deck'

# The duel: B opens the turn menu (END TURN only: never concedes); A on your deck has CONCEDE, which asks first.
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
mp cards state
press B 2
wait 6
press B 2
wait 6
rendershot turnmenu
press B 2
wait 6
mp cards state
mp cards focus 3 0
wait 6
rendershot deckhover
press A 2
wait 6
rendershot deckmenu
press A 2
wait 6
rendershot concede
press B 2
wait 6
mp cards state
press B 2
wait 6
press A 2
wait 6
press A 2
wait 3
mp cards state
quit
SCRIPT
run duel
expect duel 'turn menu: END TURN'
expect duel 'deck menu: CONCEDE'
refuse duel 'turn menu: END TURN, CONCEDE'
expect duel 'board asks: Concede this duel'
refuse duel 'board asks: End your turn?'
expect duel 'board asks: End your turn without attaching Energy'
test "$(grep -c 'screen 10 opted 1 phase 1 current 0' "$OUT/duel/log.txt")" -ge 3
expect duel 'screen 10 opted 1 phase 1 current 1'
echo 'PASS the turn menu: B opens it on END TURN, A on the deck has CONCEDE (it asks first), END TURN ends the turn (only the Energy reminder asks)'

# Nothing left to play from the hand: the cursor goes to your Active Pokemon (seed 2: you go first, nothing damaged). A hand of only a POTION (nothing
# damaged) is done; with a Fire Energy still to attach it isn't.
cat > "$OUT/done.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards fast
mp cards seed 2
mp cards open
wait 20
press A 2
wait 200
mp cards ready
wait 300
mp cards state
mp cards discardhand
mp cards hand 2
mp cards focus 0 0
wait 20
mp cards checkdone
wait 20
mp cards state
mp cards discardhand
mp cards hand 221
mp cards focus 0 0
wait 20
mp cards checkdone
wait 20
rendershot done
quit
SCRIPT
run done
test "$(grep -c 'refocus: nothing left to play, your Active' "$OUT/done/log.txt")" = 1 || { echo "FAIL done: the cursor didn't go to the Active once (only after the POTION)"; exit 1; }
echo 'PASS nothing left to play: the cursor goes to your Active Pokemon (not while Energy can still be attached)'

# An attack with a choice from a menu: Clefairy's Metronome copies Machop's Low Kick (ATTACK, Metronome, Low Kick)
cat > "$OUT/metronome.txt" <<'SCRIPT'
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
mp cards clear
mp cards put 0 0 171
mp cards attach 0 0 3 3
mp cards put 1 0 125
mp cards attach 1 0 5 2
wait 30
mp cards focus 1 0
wait 10
press A 2
wait 10
press A 2
wait 10
press DOWN 2
wait 6
press A 2
wait 10
rendershot metronome
press A 2
wait 120
mp cards state
quit
SCRIPT
run metronome
expect metronome 'attack menu: 2 choices'
expect metronome 'metronome copies Low Kick'
expect metronome 'screen 10 opted 1 phase 1 current 1'
echo "PASS an attack's menu: Metronome copies the Defending Pokemon's attack"

# The deck builder: take a CHARMANDER out of the deck (the list, then SELECT on the cards), add it back;
# copy the deck, delete the one you duel with (the copy takes over).
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
press SELECT 2
wait 4
mp cards deck 0
rendershot builder_select
press A 2
wait 4
mp cards deck 0
press B 2
wait 10
press START 2
wait 6
press A 2
wait 10
mp cards deck 1
press A 2
wait 6
press SELECT 2
wait 6
rendershot deck_delete
press A 2
wait 10
rendershot deck_deleted
mp cards deck 0
quit
SCRIPT
run builder
test "$(grep -c 'deck 0 count 60 valid 1' "$OUT/builder/log.txt")" -eq 3
test "$(grep -c 'deck 0 count 59 valid 0' "$OUT/builder/log.txt")" -eq 2
expect builder 'deck 1 count 60 valid 1'
expect builder 'deck 0 count 0 valid 0 active 1'
echo 'PASS the deck builder: out of the deck from the list and with SELECT, back in; deleting the deck you duel with'

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
mp cards points 50
press R 2
wait 4
press A 2
wait 4
press A 2
wait 10
press A 2
wait 4
mp cards state
mp cards recipes
quit
SCRIPT
run counter
expect counter 'screen 12 opted 1 .* points 10 packs 10000000000 '
expect counter 'recipes 3: \[Ember Friends\] \[Tidal Friends\] \[Garden Friends\]$'   # (structure decks only once bought)
echo 'PASS the counter: a spare card traded in, a booster pack bought; DECK RECIPES without the decks not bought'
# A PRIZES fanfare at the counter, then the case closed before it ends: the Pokemon Center's
# music plays again (it stayed paused, and a door then never finished), and the door works.
cat > "$OUT/fanfare.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards giveall
mp cards club
wait 20
press A 2
wait 4
press A 2
wait 10
press A 2
wait 4
press R 2
wait 4
press R 2
wait 4
press A 2
wait 10
mp music
press A 2
wait 6
press B 2
wait 6
press B 2
wait 6
press B 2
waitfor field 600
wait 30
mp music
press DOWN 40
wait 300
mp where
quit
SCRIPT
run fanfare 2,2,7,6
expect fanfare 'music 400 saved 0 status 8.* fanfare 1'
expect fanfare 'music 400 saved 0 status 0.* fanfare 0'
expect fanfare 'where map 0.10'
echo 'PASS a counter prize fanfare cut short: the music plays again and the door works'

# Pokemon Powers on the duel table: RAIN DANCE, ENERGY TRANS, PEEK, CURSE, CLAIRVOYANCE's
# look at their hand and a Clefairy Doll discarded, on a staged board.
cat > "$OUT/powers.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards fast
mp cards open
wait 20
press A 2
wait 200
mp cards ready
wait 200
mp cards clear
mp cards put 0 0 65
mp cards put 0 0 66
mp cards put 0 0 67
mp cards put 0 1 8
mp cards put 0 1 9
mp cards put 0 1 11
mp cards attach 0 1 1 2
mp cards put 0 2 123
mp cards put 0 3 148
mp cards put 0 3 151
mp cards put 0 3 152
mp cards put 0 4 204
mp cards put 0 4 92
mp cards put 0 5 203
mp cards hand 3
mp cards put 1 0 167
mp cards put 1 1 167
mp cards damage 1 0 10
wait 120
rendershot staged
mp cards focus 1 0
wait 4
press A 2
wait 6
press DOWN 2
wait 4
press DOWN 2
wait 4
rendershot raindance_menu
press A 2
wait 6
rendershot raindance_target
press A 2
wait 8
rendershot raindance_banner
wait 120
mp cards state
mp cards focus 1 1
wait 4
press A 2
wait 6
press A 2
wait 6
rendershot trans_from
press A 2
wait 6
rendershot trans_to
press A 2
wait 120
mp cards state
mp cards focus 1 2
wait 4
press A 2
wait 6
press A 2
wait 6
rendershot peek_menu
press DOWN 2
wait 4
press A 2
wait 52
rendershot peek_card
wait 100
mp cards focus 1 3
wait 4
press A 2
wait 6
press A 2
wait 6
rendershot curse_from
press A 2
wait 6
rendershot curse_to
press A 2
wait 150
rendershot cursed
mp cards focus 9 0
wait 4
press A 2
wait 10
rendershot clairvoyance
press B 2
wait 6
mp cards focus 1 5
wait 4
press A 2
wait 6
rendershot doll_menu
press A 2
wait 150
rendershot doll_gone
mp cards state
mp cards snapshot
quit
SCRIPT
run powers
expect powers 'conserved 1'
expect powers 'show power RAIN DANCE player 0 slot 0'
expect powers 'show power ENERGY TRANS player 0 slot 1'
expect powers 'show power PEEK player 0 slot 2'
expect powers 'show power CURSE player 0 slot 3'
expect powers 'field 3 bench 31320'
echo 'PASS Pokemon Powers on the duel table: Rain Dance, Energy Trans, Peek, Curse, Clairvoyance, a Clefairy Doll discarded'

# A pack that brings the set's last card: every COLOSSEUM card but SCYTHER (a holo), and 19 packs
# without a new holo, so this one has it: NEW, HOLO!, then COLOSSEUM COMPLETE!.
cat > "$OUT/holo.txt" <<'SCRIPT'
pressuntil A field 5000
wait 100
mp cards starter 0
mp cards giveall
mp cards own 46 0
mp cards drought 0 19
mp cards packs
wait 20
press A 2
wait 40
press B 2
wait 30
rendershot holo_open
press A 2
wait 60
rendershot holo_complete
mp cards state
quit
SCRIPT
run holo
expect holo 'opened pack 0'
expect holo 'set complete: 0'
echo 'PASS a holo the set was missing: NEW, HOLO!, the set complete'
# HOW TO PLAY: SELECT on the case's first screen, every page, B: back on the same screen (the case still open)
cat > "$OUT/guide.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp cards starter 0
mp cards case
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
wait 60
rendershot guide_last
press LEFT 2
wait 10
press B 4
wait 30
mp cards state
rendershot guide_closed
quit
SCRIPT
run guide
expect guide 'HOW TO PLAY opened (game 0, 9 pages)'
expect guide 'page 9: CONTROLS'
expect guide 'guide\] closed'
expect guide 'tcg\] screen 1 '
if grep -q "words don't fit" "$OUT/guide/log.txt"; then echo "FAIL guide: a page's words don't fit"; exit 1; fi
echo 'PASS HOW TO PLAY: every page, then back on the case'
echo "Screenshots and logs: $OUT"
