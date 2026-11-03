#!/bin/bash
# Moemon Maker's ENCOUNTERS panel (maker/maker_wild.cpp) and the game using its slots (src/maker/maker_overrides.c
# Maker_WildInfo, over mp_region.c's GENERATION pools): ROUTE 101's list with WURMPLE at 50% (spread over the fixed
# slots), then every grass slot PIKACHU Lv 5; saved; a game there rolls only PIKACHU (mp wildroll: the real encounter
# path), with GENERATION 3 and 9. Pictures in /tmp/maker_wild_test.
# usage: tools/test/maker_wild.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_wild_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_ROUTE101\nmaker mode 3\nwait 4\n'
    printf 'maker wildchance land SPECIES_WURMPLE 50\nwait 3\nmakershot list\n'
    for s in 0 1 2 3 4 5 6 7 8 9 10 11; do printf 'maker wild land %d SPECIES_PIKACHU 5 5\n' $s; done
    printf 'wait 3\nmakershot pikachu\nmaker save\nmaker close\nwait 60\nquit\n'
} > "$T/edit.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
F="$T/mm/base/MAP_ROUTE101.txt"
grep -q "wild land list: WURMPLE=50%" "$T/edit.log" || { echo "FAIL: the list didn't spread WURMPLE to 50%"; fail=1; }
grep -q "^wild land 0 species SPECIES_[A-Z]* SPECIES_PIKACHU" "$F" || { echo "FAIL: no slot edit in the file"; fail=1; }
grep -q "^wild land 11 max [0-9]* 5" "$F" || { echo "FAIL: no level edit in the file"; fail=1; }
for gen in 3 9; do
    printf 'generation = %s\n' $gen > "$T/play$gen.ini"
    printf 'pressuntil A field 5000\nwait 30\nmp wildroll 30\nquit\n' > "$T/play.txt"
    (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
        MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play$gen.ini" MOEMON_DEBUG_QUICKSTART=1 \
        MOEMON_DEBUG_WARP=0,16,10,10 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play$gen.sav" timeout 150 "$BIN" > "$T/play$gen.log" 2>&1)
    line=$(grep "\[test\] wildroll:" "$T/play$gen.log")
    echo "$line" | grep -qE "wildroll:( 25){30}$" || { echo "FAIL (GENERATION $gen): not all PIKACHU: $line"; fail=1; }
done
[ $fail = 0 ] && echo "maker_wild: OK"
exit $fail
