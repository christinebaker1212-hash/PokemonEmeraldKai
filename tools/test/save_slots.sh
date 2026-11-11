#!/bin/bash
# The saves folder and the twelve save files (src/platform/save_slots.c), in a scratch game folder:
#   1. a save made the old way (pokeemerald.sav and its files beside the game) moves into saves/slot1 unchanged
#   2. the title screen's SAVE FILE item: RIGHT goes on with file 2 (empty) at once (its save read, the menu shown
#      again on it; its own copy of the shared settings, moemon_controls.ini here)
#   3. the next start is on saves/slot2: no save there (NEW GAME); LEFT goes back to file 1 and its player
#   4. ERASE (SELECT on SAVE FILE, A twice) empties file 1 (the one shown, so the one in use) and nothing else
#   5. LEFT from file 1 goes round to file 12
#   6. settings follow File 1 (the user, 2026-11-11): file 2 starts with none of its own and reads File 1's; a line it
#      changed stays its own; a change in File 1 reaches it where it changed nothing; A on SAVE FILE asks to ERASE too
#   7. a brand-new game (nothing beside it): File 1's settings are the canonical list, and a new game starts with them
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
[ -f "$T/saves/slot2/moemon_controls.ini" ] || { echo "FAIL: save file 2 has no settings to read"; fail=1; }
grep -q "^rumble = 0" "$T/saves/slot2/moemon_controls.ini" || { echo "FAIL: file 2 doesn't read File 1's settings"; fail=1; }
grep -q "^[a-z]" "$T/saves/slot2/moemon_controls.ini.own" 2>/dev/null && { echo "FAIL: file 2 has settings of its own already"; fail=1; }
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
press A 2
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
# 6. file 2's own change stays; File 1's change reaches it where it changed nothing
printf 'rumble = 1\n' > "$T/saves/slot2/moemon_controls.ini.own"
printf 'layout = standard\nanalog_run = 1\nrumble = 0\ncontroller = all\nbutton_icons = auto\n' > "$T/saves/slot1/moemon_controls.ini"
echo 1 > "$T/saves/slot.txt"
run inherit switch.txt   # (from file 1 RIGHT to file 2: its settings read again)
grep -q "^rumble = 1" "$T/saves/slot2/moemon_controls.ini" || { echo "FAIL: file 2 lost its own RUMBLE"; fail=1; }
grep -q "^analog_run = 1" "$T/saves/slot2/moemon_controls.ini" || { echo "FAIL: File 1's change didn't reach file 2"; fail=1; }
# 7. a brand-new game: no settings anywhere
N=$T/new; mkdir -p "$N"
printf 'pressuntil A field 5000\nwait 30\nquit\n' > "$N/new.txt"
(cd "$N" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$N/new.txt" MOEMON_SHOTDIR="$N" \
    MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_QUICKSTART=1 timeout 150 "$BIN" > "$N/new.log" 2>&1)
# (tests never start the controllers, so nothing asks for moemon_controls.ini here; when it's there it's the list's)
[ ! -f "$N/saves/slot1/moemon_controls.ini" ] || grep -q "^layout = nintendo" "$N/saves/slot1/moemon_controls.ini" \
    || { echo "FAIL: a new game's controls aren't the list's"; fail=1; }
grep -q "^window_scale = 4" "$N/saves/slot1/moemon_display.ini" 2>/dev/null || { echo "FAIL: a new game's display isn't the list's"; fail=1; }
for k in "text_speed = fast" "generation = 4" "battle_anims = modern" "gimmicks = 168" "trainer_range = 5" "spectate_size = 2"; do
    grep -q "^$k" "$N/saves/slot1/moemon_coop.ini" 2>/dev/null || { echo "FAIL: a new game's settings lack '$k'"; fail=1; }
done
grep -q "options: text speed 2, frame 1, sound 1, battle scene 1, style 0, name case 0, creatures 0" "$N/new.log" \
    || { echo "FAIL: the new game didn't start with the list's options"; grep "options:" "$N/new.log"; fail=1; }
for f in "$T"/*.bmp; do [ -f "$f" ] && python3 -c "from PIL import Image; import sys; Image.open(sys.argv[1]).save(sys.argv[1][:-4]+'.png')" "$f" && rm -f "$f"; done
rm -rf /tmp/save_slots_test; mv "$T" /tmp/save_slots_test
[ $fail = 0 ] && echo "PASS save files: the old save moved in unchanged, file 2 and back to 1 on the title screen, file 1 erased (A), round to 12, settings following File 1, a new game's own"
exit $fail
