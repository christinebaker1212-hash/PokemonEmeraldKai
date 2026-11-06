#!/bin/bash
# Moemon Maker M4b: the decompiler's round trip (decision 19). tools/maker/decompile.py --proof writes every label
# block of the HOENN scripts as an event of raw lines; the game compiles each (src/maker/maker_raw.c: the game's own
# macros at run time) and walks it beside the built script ("mp makerproof": the same bytes; a pointer into the
# event the same place as the game's label of that name; any other pointer the same address).
# usage: tools/test/maker_decompile.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_decompile_test
rm -rf "$T"; mkdir -p "$T/shots"
python3 "$(dirname "$0")/../maker/decompile.py" --proof "$T/proof.txt" || exit 1
cat > "$T/play.txt" <<S
pressuntil A field 5000
wait 10
mp makerproof $T/proof.txt
wait 10
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_SAVE="$T/play.sav" timeout 400 "$BIN" > "$T/play.log" 2>&1)
line=$(grep "maker proof: [0-9]* blocks" "$T/play.log")
echo "${line:-no result}"
grep "maker proof: .*: " "$T/play.log" | head -40
case "$line" in *", 0 differ") echo "maker_decompile: OK"; exit 0;; esac
exit 1
