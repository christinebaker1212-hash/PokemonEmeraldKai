#!/bin/bash
# SEARCH & SORT in the three card games' binders and deck builders, with real keys: {SEARCH} opens it, A on SEARCH
# types a name (the keyboard), the list follows, add / take out / INSPECT / paging still work on the narrowed list,
# an empty result comes back with CLEAR ALL, and each game's settings are kept apart in "<save>.search".
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/card-search}
BIN=${MAIN_BIN:-$REPO/pokeemerald64}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
run() {
    local name=$1
    rm -rf "${OUT:?}/$name"; mkdir -p "$OUT/$name"
    printf '[player]\nAllowSmallTweaks = 1\nautosave = 0\n' > "$OUT/$name/player.ini"
    printf 'window_scale = 2\n' > "$OUT/$name/display.ini"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_WARP=0,10,8,10 \
        MOEMON_TEST="$OUT/$name.txt" MOEMON_SHOTDIR="$OUT/$name" \
        MOEMON_COOP_CONFIG="$OUT/$name/player.ini" MOEMON_DISPLAY_CONFIG="$OUT/$name/display.ini" \
        MOEMON_CONTROLS_CONFIG="$OUT/$name/controls.ini" MOEMON_SAVE="$OUT/$name/player.sav" \
        timeout -k 5 ${TIMEOUT:-150} "$BIN" > "$OUT/$name/log.txt" 2>&1
}
# the Nth "shown" line of a game's log (1-based)
shown() { grep "\] shown " "$OUT/$1/log.txt" | sed -n "$2p"; }
want() {   # want NAME N REGEX: the Nth shown line matches
    local line; line=$(shown "$1" "$2")
    [[ "$line" =~ $3 ]] || { echo "FAIL $1 shown #$2: '$line' !~ '$3'"; exit 1; }
}
deck() { shown "$1" "$2" | sed -n 's/.* deck \([0-9]*\)).*/\1/p'; }
expect() { grep -q -- "$2" "$OUT/$1/log.txt" || { echo "FAIL $1: no '$2'"; exit 1; }; }

# The flow every game runs: the builder, {SEARCH}, A on SEARCH, type, Enter, B; take out, ADD, INSPECT and back,
# page down; then nothing found ("zzzz") and CLEAR ALL from the menu.
flow() {   # flow PREFIX WORD TAKEOUT_KEY
    cat <<SCRIPT
wait 20
$1 shown
press SEARCH 2
wait 20
press A 2
wait 10
typetext $2\n
wait 10
$1 shown
rendershot typed
press B 2
wait 10
$1 shown
press $3 2
wait 10
$1 shown
press A 2
wait 10
$1 shown
press INSPECT 2
wait 30
rendershot inspect
press INSPECT 2
wait 30
$1 shown
$1 search clear
wait 5
mash DOWN 4 8
wait 10
$1 shown
rendershot paged
press SEARCH 2
wait 20
press A 2
wait 10
typetext zzzz\n
wait 10
press B 2
wait 10
$1 shown
rendershot empty
press SEARCH 2
wait 20
press UP 2
wait 6
press UP 2
wait 6
press A 2
wait 10
press B 2
wait 10
$1 shown
SCRIPT
}

# Pokémon: the builder ("char": 3 cards), then the binder sorted by HP, highest first
cat > "$OUT/tcg.txt" <<SCRIPT
pressuntil A field 5000
wait 60
mp cards starter 0
mp cards giveall
mp cards search clear
mp cards builder
$(flow "mp cards" char SELECT)
mp cards search filter 1 2
mp cards search sort 4
mp cards search desc
wait 5
mp cards shown
mp cards search clear
quit
SCRIPT
run tcg
want tcg 2 "shown 3 "
want tcg 3 "shown 3 "
[ "$(deck tcg 4)" = $(( $(deck tcg 3) - 1 )) ] || { echo "FAIL tcg: SELECT didn't take it out ($(deck tcg 3) -> $(deck tcg 4))"; exit 1; }
[ "$(deck tcg 5)" = "$(deck tcg 3)" ] || { echo "FAIL tcg: ADD didn't put it back"; exit 1; }
c5=$(shown tcg 5 | sed 's/.* card \([0-9]*\) .*/\1/'); c6=$(shown tcg 6 | sed 's/.* card \([0-9]*\) .*/\1/')
[ "$c5" = "$c6" ] || { echo "FAIL tcg: INSPECT came back to another card ($c5 / $c6)"; exit 1; }
want tcg 7 "cursor ([4-9]|[1-9][0-9])"
want tcg 8 "shown 0 "
want tcg 9 "shown [1-9][0-9]* "
expect tcg "\[search\] clear all"
want tcg 10 "shown [1-9][0-9]* .*: Charizard"
echo "PASS Pokémon: type, ADD / take out, INSPECT, paging, empty + CLEAR ALL, sorted by HP"

# Yu-Gi-Oh!
cat > "$OUT/ygo.txt" <<SCRIPT
pressuntil A field 5000
wait 60
mp ygo starter 0
mp ygo giveall
mp ygo search clear
mp ygo builder
$(flow "mp ygo" "dark magician" SELECT)
mp ygo binder
mp ygo search filter 0 1
mp ygo search filter 2 2
mp ygo search sort 5
mp ygo search desc
wait 5
mp ygo shown
mp ygo search clear
mp ygo search filter 8 2
wait 5
mp ygo shown
mp ygo search clear
quit
SCRIPT
run ygo
want ygo 2 "shown [1-9] .*: Dark Magician,"
[ "$(deck ygo 4)" = $(( $(deck ygo 3) - 1 )) ] || { echo "FAIL ygo: SELECT didn't take it out ($(deck ygo 3) -> $(deck ygo 4))"; exit 1; }
[ "$(deck ygo 5)" = "$(deck ygo 3)" ] || { echo "FAIL ygo: ADD didn't put it back"; exit 1; }
c5=$(shown ygo 5 | sed 's/.* card \([0-9]*\) .*/\1/'); c6=$(shown ygo 6 | sed 's/.* card \([0-9]*\) .*/\1/')
[ "$c5" = "$c6" ] || { echo "FAIL ygo: INSPECT came back to another card"; exit 1; }
want ygo 7 "cursor ([4-9]|[1-9][0-9])"
want ygo 8 "shown 0 "
want ygo 9 "shown [1-9][0-9]+ "
want ygo 10 "shown [1-9][0-9]* .*: Gate Guardian,"
want ygo 11 "shown [1-9][0-9]* "
echo "PASS Yu-Gi-Oh!: type, ADD / take out, INSPECT, paging, empty + CLEAR ALL, DARK by ATK, LIMITED"

# Triple Triad: the builder takes out with A (A on a card in the hand)
cat > "$OUT/tt.txt" <<SCRIPT
pressuntil A field 5000
wait 60
mp tt starter
mp tt giveall
mp tt search clear
mp tt builder
$(flow "mp tt" "bomb" A)
mp tt binder
mp tt search filter 0 1
mp tt search sort 8
mp tt search desc
wait 5
mp tt shown
press R 2
wait 10
mp tt shown
mp tt search clear
quit
SCRIPT
run tt
want tt 2 "shown [1-9] .*Bomb"
d3=$(deck tt 3); d4=$(deck tt 4); d5=$(deck tt 5)
[ "$d4" != "$d3" ] || { echo "FAIL tt: A on a card changed nothing"; exit 1; }
[ "$d5" = "$d3" ] || { echo "FAIL tt: A again didn't undo it"; exit 1; }
c5=$(shown tt 5 | sed 's/.* card \([0-9]*\) .*/\1/'); c6=$(shown tt 6 | sed 's/.* card \([0-9]*\) .*/\1/')
[ "$c5" = "$c6" ] || { echo "FAIL tt: INSPECT came back to another card"; exit 1; }
want tt 7 "cursor ([4-9]|[1-9][0-9])"
want tt 8 "shown 0 "
want tt 10 "shown 475 "
want tt 11 "shown [1-9][0-9]* "
echo "PASS Triple Triad: type, put in / take out, INSPECT, paging, empty + CLEAR ALL, every star by TOTAL, L/R back to the tabs"

# each game's own line, kept apart
for g in tcg ygo tt; do
    expect "$g" "\[search\] closed"
done
echo "PASS card search"
