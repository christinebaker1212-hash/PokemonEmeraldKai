#!/bin/bash
# Moemon Maker's way in (docs/MOEMON_MAKER.md "The way in", src/maker/maker_gate.c):
#   1. without AllowSmallTweaks = 1, typing "maker" on the main menu adds nothing
#   2. with it, typing "maker" adds MOéMON MAKER (a fifth item with CONTINUE and SAVE FILE: the menu scrolls to it)
#   3. the next start doesn't remember it
# usage: tools/test/maker_gate.sh [BIN]   (results in /tmp/maker_gate_test)
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_gate_test
rm -rf "$T"; mkdir -p "$T"
fail=0
run() {   # run NAME SCRIPT CONFIG [VAR=value...]
    local name=$1 script=$2 cfg=$3; shift 3
    mkdir -p "$T/$name"
    (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/$script" \
        MOEMON_SHOTDIR="$T/$name" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/$cfg" "$@" timeout 150 "$BIN" \
        > "$T/$name.log" 2>&1)
}
want() { grep -q -- "$2" "$T/$1.log" || { echo "FAIL $1: no '$2'"; fail=1; }; }
printf 'AllowSmallTweaks = 0\n' > "$T/off.ini"
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
printf 'pressuntil A field 5000\nwait 30\nmp save\nwait 60\nquit\n' > "$T/make.txt"
run make make.txt off.ini MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_NAME=MAKER
cat > "$T/menu.txt" <<'S'
pressuntil START mainmenu 3000
wait 40
shot before
typetext maker
wait 60
waitfor mainmenu 600
wait 40
shot typed
press DOWN 2
wait 8
press DOWN 2
wait 8
press DOWN 2
wait 8
press DOWN 2
wait 20
shot last
quit
S
run off menu.txt off.ini
run on menu.txt on.ini
printf 'pressuntil START mainmenu 3000\nwait 40\nshot before\nquit\n' > "$T/again.txt"
run again again.txt on.ini
same() { cmp -s "$T/$1" "$T/$2"; }
ls "$T"/on/*.* >/dev/null 2>&1 || { echo "FAIL: no screenshots"; fail=1; }
f() { ls "$T/$1/$2".* 2>/dev/null | head -1; }
same "$(f off before | sed "s|$T/||")" "$(f off typed | sed "s|$T/||")" || { echo "FAIL: without AllowSmallTweaks the menu changed"; fail=1; }
same "$(f on before | sed "s|$T/||")" "$(f on typed | sed "s|$T/||")" && { echo "FAIL: typing maker didn't change the menu"; fail=1; }
same "$(f on before | sed "s|$T/||")" "$(f again before | sed "s|$T/||")" || { echo "FAIL: the next start remembered it"; fail=1; }
grep -q "SE_SUCCESS\|maker" "$T/on.log" || true
[ $fail = 0 ] && echo "maker_gate: OK"
exit $fail
