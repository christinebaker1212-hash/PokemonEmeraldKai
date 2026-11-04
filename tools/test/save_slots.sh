#!/bin/bash
# The saves folder and the twelve save files (src/platform/save_slots.c), in a scratch game folder:
#   1. a save made the old way (pokeemerald.sav and its files beside the game) moves into saves/slot1 unchanged
#   2. the title screen's SAVE FILE item: RIGHT goes on with file 2 (empty) at once (its save read, the menu shown
#      again on it; its own copy of the shared settings, moemon_controls.ini here)
#   3. the next start is on saves/slot2: no save there (NEW GAME); LEFT goes back to file 1 and its player
#   4. ERASE (SELECT on SAVE FILE, A twice) empties file 1 (the one shown, so the one in use) and nothing else
#   5. LEFT from file 1 goes round to file 12
# usage: tools/test/save_slots.sh [BIN]   (results in /tmp/save_slots_test)
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/save_slots_test.$$
mkdir -p "$T"
fail=0
run() {   # run NAME SCRIPT [VAR=value...]: the game in $T with the script, log in $T/NAME.log
    local name=$1 script=$2; shift 2
    (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/$script" \
        MOEMON_SHOTDIR="$T" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/a.ini" "$@" timeout 150 "$BIN" \
        > "$T/$name.log" 2>&1)
}
want() { grep -q -- "$2" "$T/$1.log" || { echo "FAIL $1: no '$2'"; fail=1; }; }
printf 'pressuntil A field 5000\nwait 30\nmp save\nwait 60\nquit\n' > "$T/make.txt"
run make make.txt MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_NAME=SLOTONE MOEMON_SAVE="$T/pokeemerald.sav"
(cd "$T" && md5sum pokeemerald.sav* | sort) > "$T/before.md5"
cat > "$T/switch.txt" <<'S'
pressuntil START mainmenu 3000
wait 30
shot menu1
press DOWN 2
wait 10
press DOWN 2
wait 10
press DOWN 2
wait 10
press RIGHT 2
wait 60
waitfor mainmenu 1200
wait 30
shot menu1_file2
quit
S
printf 'rumble = 0\n' > "$T/moemon_controls.ini"
run switch switch.txt
want switch "moved [0-9]* save file(s) into saves/slot1"
want switch "save file 1: saves/slot1/pokeemerald.sav"
want switch "now on save file 2"
want switch "reached state"
[ -f "$T/saves/slot2/moemon_controls.ini" ] || { echo "FAIL: save file 2 has no settings of its own"; fail=1; }
[ -f "$T/pokeemerald.sav" ] && { echo "FAIL: the old save is still beside the game"; fail=1; }
(cd "$T/saves/slot1" && md5sum pokeemerald.sav* | sort) | diff -q - "$T/before.md5" >/dev/null \
    || { echo "FAIL: the moved files changed"; fail=1; }
grep -qx 2 "$T/saves/slot.txt" || { echo "FAIL: saves/slot.txt isn't 2"; fail=1; }
cat > "$T/second.txt" <<'S'
pressuntil START mainmenu 3000
wait 30
shot menu2
press DOWN 2
wait 10
press DOWN 2
wait 10
press LEFT 2
wait 60
waitfor mainmenu 1200
wait 30
shot menu2_file1
press SELECT 2
wait 60
shot erase_ask
press A 2
wait 60
shot erase_ask2
press A 2
wait 120
shot erased
quit
S
run second second.txt
want second "save file 2: saves/slot2/pokeemerald.sav"
want second "now on save file 1"
want second "erased save file 1"
# (file 1 is the one in use: it's open again, empty: no save in it, nothing of its POKéMON's side files)
[ -s "$T/saves/slot1/pokeemerald.sav" ] && { echo "FAIL: file 1 wasn't erased"; fail=1; }
ls "$T/saves/slot1" | grep -v -x -e pokeemerald.sav -e pokeemerald.sav.pals -e moemon_controls.ini -e 'moemon_.*\.ini' \
    | grep -q . && { echo "FAIL: files left in saves/slot1: $(ls "$T/saves/slot1")"; fail=1; }
cat > "$T/round.txt" <<'S'
pressuntil START mainmenu 3000
wait 30
press DOWN 2
wait 10
press DOWN 2
wait 10
press LEFT 2
wait 60
waitfor mainmenu 1200
wait 30
shot menu_file12
quit
S
run round round.txt
want round "now on save file 12"
grep -qx 12 "$T/saves/slot.txt" || { echo "FAIL: saves/slot.txt isn't 12"; fail=1; }
[ -d "$T/saves/slot2" ] || { echo "FAIL: no saves/slot2"; fail=1; }
for f in "$T"/*.bmp; do [ -f "$f" ] && python3 -c "from PIL import Image; import sys; Image.open(sys.argv[1]).save(sys.argv[1][:-4]+'.png')" "$f" && rm -f "$f"; done
rm -rf /tmp/save_slots_test; mv "$T" /tmp/save_slots_test
[ $fail = 0 ] && echo "PASS save files: the old save moved in unchanged, file 2 and back to 1 on the title screen, file 1 erased, round to 12"
exit $fail
