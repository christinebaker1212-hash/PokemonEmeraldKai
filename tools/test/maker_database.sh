#!/bin/bash
# Moemon Maker M3a's database (src/maker/maker_database.c, maker/maker_database_ui.cpp): the editor changes TREECKO's
# stats, types, name and learnset, POUND's power and POTION's price and name (MoemonMaker/database.txt, each line with
# the game's own value) and opens the DATABASE window; a game then reads the edited values, and an edit whose "was" no
# longer matches the game is left out.
# usage: tools/test/maker_database.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_database_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
# a picture of the user's own for TREECKO's front: a red disc on a clear background
mkdir -p "$T/mm/art"
python3 - "$T/mm/art/SPECIES_TREECKO_front.png" <<'PY'
import sys, zlib, struct
rows = b''
for y in range(64):
    rows += b'\0' + b''.join(bytes((220, 30, 30, 255)) if (x - 32) ** 2 + (y - 32) ** 2 < 400 else bytes((0, 0, 0, 0)) for x in range(64))
def chunk(t, d):
    return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 64, 64, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b'')
open(sys.argv[1], 'wb').write(png)
PY
cat > "$T/edit.txt" <<'S'
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
maker db species SPECIES_TREECKO stats 50 55 45 80 75 65
maker db species SPECIES_TREECKO types TYPE_GRASS TYPE_DRAGON
maker db species SPECIES_TREECKO name "GECKO"
maker db species SPECIES_TREECKO learnset 1:MOVE_DRAGON_RAGE 5:MOVE_ABSORB
maker db move MOVE_POUND power 55
maker db item ITEM_POTION price 250
maker db item ITEM_POTION name "TONIC"
maker database SPECIES_TREECKO
maker pic SPECIES_TREECKO
wait 6
makershot database
maker close
wait 60
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
want() { grep -q -- "$2" "$1" 2>/dev/null || { echo "FAIL: no '$2' in $(basename "$1")"; fail=1; }; }
want "$T/edit.log" "\[maker\] art: 1 POKeMON with pictures of the user's own"
want "$T/edit.log" "\[maker\] picture SPECIES_TREECKO middle 2[0-9][0-9],[0-9]*,[0-9]*,255 corner alpha 0"
want "$T/mm/database.txt" "^species SPECIES_TREECKO"
want "$T/mm/database.txt" "^  stats 50 55 45 80 75 65 was=40 45 35 70 65 55"
want "$T/mm/database.txt" "^  types TYPE_GRASS TYPE_DRAGON was=TYPE_GRASS TYPE_GRASS"
want "$T/mm/database.txt" "^  learnset 1:MOVE_DRAGON_RAGE 5:MOVE_ABSORB was=0x"
want "$T/mm/database.txt" "^move MOVE_POUND"
want "$T/mm/database.txt" "^  power 55 was=40"
want "$T/mm/database.txt" "^  price 250 was=300"
play() {
    printf 'pressuntil A field 5000\nwait 20\nmp dbinfo 277 1 13\nquit\n' > "$T/play.txt"
    (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
        MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
        MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 150 "$BIN" > "$T/$1" 2>&1)
}
play play.log
want "$T/play.log" "\[maker\] database: 3 edited entries"
want "$T/play.log" "db species 277 GECKO stats 50 55 45 80 75 65 types 12 16 learn0 82"
want "$T/play.log" "db move 1 POUND power 55 accuracy 100"
want "$T/play.log" "db item 13 TONIC price 250"
grep -q "left out" "$T/play.log" && { echo "FAIL: an edit was left out"; grep "left out" "$T/play.log"; fail=1; }
# an edit made against an older game (its "was" no longer the game's): left out, the game's own value plays
sed -i 's/^  power 55 was=40/  power 55 was=35/' "$T/mm/database.txt"
play stale.log
want "$T/stale.log" "database: MOVE_POUND power changed in the game since the edit (left out)"
want "$T/stale.log" "db move 1 POUND power 40 "
want "$T/stale.log" "db item 13 TONIC price 250"
[ $fail = 0 ] && echo "maker_database: OK"
exit $fail
