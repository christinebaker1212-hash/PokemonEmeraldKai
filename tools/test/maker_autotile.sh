#!/bin/bash
# Moemon Maker's AUTOTILE (src/maker/maker_autotile.c): the terrains learned from the game's own maps of ROUTE 101's
# tilesets; a pond painted with the rectangle (its rim on top and sides, open water on its bottom row, the grass lip
# under it, as ROUTE 117's ponds), a bite erased back to the ground (the rim drawn round what's left), a sand path two
# tiles thick (its top and bottom edges, its corners), a stand of trees (whole 2x2 trees); one undo per stroke; how
# much of ROUTE 101's terrain the learned rules draw again the same; and a fix of the user's (TERRAINS: the pond's
# top-left corner as another tile) kept in MoemonMaker/autotile/. Pictures in /tmp/maker_autotile_test.
# usage: tools/test/maker_autotile.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_autotile_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1600 960\nmaker open MAP_ROUTE101\nwait 4\nmaker zoom 2\n'
    printf 'maker autotile on\nmaker autoredraw\n'
    # the pond: 5 x 4 on the grass at 4, 5
    printf 'maker terrain 0x0A1\nmaker autorect 4 5 8 8\nmaker dumpblocks 3 4 9 9\n'
    # a bite out of it, back to the ground
    printf 'maker erasebase GROUND\nmaker autopaint 8 5\nmaker autopaint 8 6\nmaker dumpblocks 3 4 9 9\n'
    # the sand path, two thick, and the trees
    printf 'maker terrain 0x121\nmaker autorect 2 16 15 17\nmaker autorect 2 11 3 15\n'
    printf 'maker terrain TREES\nmaker autorect 12 11 15 13\nmaker dumpblocks 11 10 16 14\n'
    printf 'wait 4\nmakershot route101\nmaker undo\nmaker dumpblocks 11 10 16 14\nmaker redo\n'
    printf 'maker tileset terrains\nmaker tileset terrain 0x0A1\nmaker tileset slot 0x0A1 0 0x0A0\nwait 4\nmakershot terrains\n'
    # the fix used: a new pond's top-left corner is the fixed tile
    printf 'maker terrain 0x0A1\nmaker autorect 14 2 17 4\nmaker dumpblocks 14 2 17 2\n'
    # an autotile of the user's own, RPG Maker's way: its 3x3 block from 0x0F0 on the sheet, painted 4 x 3
    printf 'maker tileset newblock 0x0F0\nwait 4\nmakershot ownautotile\nmaker autotile on\nmaker terrain 0x0F9\nmaker autorect 1 1 4 3\nmaker dumpblocks 1 1 4 3\n'
    printf 'maker save\nwait 2\nquit\n'
} > "$T/s.txt"
(cd "$ROOT" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/s.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/s.sav" MOEMON_MAKER_ATDEBUG=${ATDEBUG:-} timeout 240 "$BIN" > "$T/run.log" 2>&1)
L="$T/run.log"
grep -a "autotile\|row " "$L" | grep -v "autotile [A-Z]" | head -60 > "$T/rows.txt"
row() { grep -a "row $1:" "$L" | sed -n "${2}p" | sed 's/.*row [0-9]*://'; }
last() { grep -a "row $1:" "$L" | tail -1 | sed 's/.*row [0-9]*://'; }
grep -q "autotile: General+Petalburg, [0-9]* terrains learned" "$L" || { echo "FAIL: no terrains learned"; fail=1; }
r=$(grep -a "autotile redraw MAP_ROUTE101" "$L" | grep -o "[0-9]* per mille" | cut -d' ' -f1)
[ "${r:-0}" -ge 850 ] || { echo "FAIL: the learned rules draw only ${r:-?} per mille of ROUTE 101 again"; fail=1; }
# the pond: its rim on top (0x0B0-0x0B2 corners and top) and sides (0x0B8 / 0x0BA), open water, the lip (0x002) under it
[[ "$(row 5 1)" == *"0B1 0B1 0B1"*"0B2"* ]] || { echo "FAIL: the pond's top: $(row 5 1)"; fail=1; }
[[ "$(row 7 1)" == *"0B8 0A1 0A1 0A1 0BA"* ]] || { echo "FAIL: the pond's sides: $(row 7 1)"; fail=1; }
[[ "$(row 9 1)" == *"002 002"* ]] || { echo "FAIL: no lip under the pond: $(row 9 1)"; fail=1; }
# the bite: the rim drawn round what's left
[[ "$(row 6 2)" == *"0A1 0A1 0BA 001"* ]] || { echo "FAIL: the bite's rim: $(row 6 2)"; fail=1; }
# whole trees (left and right halves side by side), and the stroke undone in one step
[[ "$(row 12 1)" == *"1D4 1D5 1D4 1D5"* ]] || { echo "FAIL: not whole trees: $(row 12 1)"; fail=1; }
[[ "$(row 12 2)" == *"00D 00D 00D 00D"* ]] || { echo "FAIL: the trees weren't undone in one step: $(row 12 2)"; fail=1; }
# the user's fix: kept in MoemonMaker/autotile/ and used
grep -q "^fix 0x0A1 208 0 0x0A0" "$T/mm/autotile/General+Petalburg.txt" 2>/dev/null || { echo "FAIL: the fix isn't in MoemonMaker/autotile/"; fail=1; }
[[ "$(row 2 1)" == " 0A0"* ]] || { echo "FAIL: the fixed corner isn't used: $(row 2 1)"; fail=1; }
# (rows 1-3, the user's own: corners, edges and centres from its 3x3 block)
[[ "$(last 1)" == " 0F0 0F1 0F1 0F2" ]] || { echo "FAIL: the own autotile's top: $(last 1)"; fail=1; }
[[ "$(last 2)" == " 0F8 0F9 0F9 0FA" ]] || { echo "FAIL: the own autotile's middle: $(last 2)"; fail=1; }
[[ "$(last 3)" == " 100 101 101 102" ]] || { echo "FAIL: the own autotile's bottom: $(last 3)"; fail=1; }
grep -q "^terrain 0x0F9" "$T/mm/autotile/General+Petalburg.txt" 2>/dev/null || { echo "FAIL: the own autotile isn't kept"; fail=1; }
for s in route101 terrains; do grep -q "shot .*$s.bmp ok" "$L" || { echo "FAIL: no picture $s"; fail=1; }; done
[ $fail = 0 ] && echo "maker_autotile: OK (redraw $r per mille)"
exit $fail
