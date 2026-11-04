#!/bin/bash
# Moemon Maker's editor window (maker/maker_ui.cpp): opened from the main menu's MOéMON MAKER, a map picked in MAPS,
# both looks, closed again (back to the main menu). Pictures in /tmp/maker_shell_test.
# usage: tools/test/maker_shell.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_shell_test
rm -rf "$T"; mkdir -p "$T"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/shell.txt" <<'S'
pressuntil START mainmenu 3000
wait 40
press DOWN 2
wait 8
press DOWN 2
wait 8
press DOWN 2
wait 8
press DOWN 2
wait 8
press DOWN 2
wait 8
press A 2
wait 60
maker size 1280 800
wait 3
maker open MAP_ROUTE101
wait 5
makershot classic
wait 3
maker find littleroot
wait 3
makershot find
wait 2
maker find
maker theme dark
wait 3
makershot dark
wait 2
maker theme classic
maker close
wait 60
waitfor mainmenu 600
wait 20
shot menu_after
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/shell.txt" \
    MOEMON_SHOTDIR="$T" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    timeout 150 "$BIN" > "$T/shell.log" 2>&1)
want() { grep -q -- "$1" "$T/shell.log" || { echo "FAIL: no '$1'"; fail=1; }; }
want "\[maker\] opened"
want "\[maker\] shot .*classic.bmp ok"
want "\[maker\] shot .*dark.bmp ok"
want "\[maker\] closed"
want "reached state"
[ $fail = 0 ] && echo "maker_shell: OK"
exit $fail
