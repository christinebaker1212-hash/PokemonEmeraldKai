#!/bin/bash
# Moemon Maker M5's tileset editor (maker/maker_tilesets_ui.cpp, src/maker/maker_tilesets.c): in LITTLEROOT TOWN the
# editor's TILESETS window picks the metatile under (10,14), gives its first bottom tile another entry and its
# behavior TALL GRASS, and saves; the game started with that file reads the new behavior there and the new entry, and
# an edit whose WAS the game doesn't have is left out (counted).
# usage: tools/test/maker_tilesets.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_tilesets_test
rm -rf "$T"; mkdir -p "$T/mm" "$T/shots"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
# which metatile is under (10,14), and its behavior before
cat > "$T/probe.txt" <<'S'
pressuntil A field 5000
wait 10
mp metatileat 10,14
mp behaviorat 10,14
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/probe.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/p.sav" timeout 120 "$BIN" > "$T/probe.log" 2>&1)
MT=$(grep -o "metatile 10,14 = 0x[0-9A-F]*" "$T/probe.log" | grep -o "0x[0-9A-F]*$")
B0=$(grep -o "behavior 10,14 = 0x[0-9A-F]*" "$T/probe.log" | grep -o "0x[0-9A-F]*$")
echo "metatile $MT, behavior $B0"
[ -n "$MT" ] || { echo "FAIL: no metatile probed"; exit 1; }
# the editor
cat > "$T/edit.txt" <<S
pressuntil START mainmenu 3000
wait 40
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press A 2
wait 60
maker size 1280 800
maker open MAP_LITTLEROOT_TOWN
wait 5
maker tileset open
maker tileset primary
maker tileset pick $MT
maker tileset entry 0 3005
maker tileset attr 0002
maker tileset value 0
maker tileset value -1
wait 10
makershot tilesets
maker tileset save
wait 5
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 200 "$BIN" > "$T/edit.log" 2>&1)
grep "tileset" "$T/edit.log" | head -5
f=$(ls "$T"/mm/tilesets/*.txt 2>/dev/null | head -1)
[ -n "$f" ] && grep -q "^attr .* 0x0002$" "$f" && grep -q "^metatile .* 0 0x[0-9A-F]* 0x3005$" "$f" \
    || { echo "FAIL: the tileset edits weren't saved"; cat "$f" 2>/dev/null; fail=1; }
grep -q "\[maker\] shot .*tilesets.bmp ok" "$T/edit.log" || { echo "FAIL: no picture"; fail=1; }
# the game reads them; an edit with a WAS the game doesn't have is left out
[ -n "$f" ] && printf 'attr 0x3FF 0x7777 0x0003\n' >> "$f"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/probe.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/p2.sav" timeout 120 "$BIN" > "$T/play.log" 2>&1)
grep -q "behavior 10,14 = 0x2$" "$T/play.log" || { echo "FAIL: the game didn't read the new behavior"; grep behavior "$T/play.log"; fail=1; }
grep -q "edits left out" "$T/play.log" || { echo "FAIL: the stale edit wasn't left out"; fail=1; }
[ $fail = 0 ] && echo "maker_tilesets: OK"
exit $fail
