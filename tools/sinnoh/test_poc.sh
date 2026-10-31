#!/bin/bash
# SINNOH's 3D engine, end to end: builds the 3D pack from a pret/pokeplatinum checkout (tools/sinnoh/fetch_decomp.sh
# makes one), then in the game
#   1. TWINLEAF TOWN in 3D at noon matches the offline reference picture (tools/sinnoh/render_ref.py) pixel for pixel
#      within rounding (the player's own spot left out)
#   2. Platinum's collision: walking up into the player's house stops at its wall
#   3. walking north out of TWINLEAF reaches ROUTE 201 (the maps' connection) with the 3D view following
#   4. ROUTE 201's east edge leads into SANDGEM TOWN (every outdoor map of the matrix is there)
#   5. the ground's height (BDHC): TWINLEAF's 16, its pond's 8, blended between tile centres
#   and, given a Platinum ROM too, that the pack built from the cartridge is the same, byte for byte
# usage: tools/sinnoh/test_poc.sh POKEPLATINUM_DIR [PLATINUM_ROM]   (BIN=the game, default ./pokeemerald64)
SRC="${1:?usage: test_poc.sh POKEPLATINUM_DIR [PLATINUM_ROM]}"
ROM="$2"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$(mktemp -d)}"
mkdir -p "$OUT"
export BIN="${BIN:-$REPO/pokeemerald64}"
fail=0
check() { if eval "$2"; then echo "ok   $1"; else echo "FAIL $1"; fail=$((fail + 1)); fi; }

python3 -I "$REPO/tools/sinnoh/build_pack.py" "$SRC" "$OUT/platinum3d.pak" > "$OUT/pack.log" || { echo "FAIL pack"; exit 1; }
if [ -n "$ROM" ]; then
    python3 -I "$REPO/tools/sinnoh/build_pack.py" --rom "$ROM" "$SRC" "$OUT/from_rom.pak" > "$OUT/pack_rom.log"
    check "the pack from the ROM is the decomp's, byte for byte" "cmp -s '$OUT/platinum3d.pak' '$OUT/from_rom.pak'"
fi
run() {   # NAME SCRIPT [VAR=value...]
    local name="$1" script="$2"
    shift 2
    mkdir -p "$OUT/$name"
    printf '%b' "$script" > "$OUT/$name/a.txt"
    TMO=120 bash "$REPO/tools/test/run1.sh" "$OUT/$name" MOEMON_S3D_PAK="$OUT/platinum3d.pak" MOEMON_S3D_HOUR=12 "$@" > /dev/null
}

run look 'pressuntil A field 5000\nwait 30\nmp warp 40,0,14,9\nwait 200\nshot frame\nquit\n'
python3 -I "$REPO/tools/sinnoh/render_ref.py" "$SRC" "$OUT/look/ref.png" 0 110 873 12 > /dev/null
check "TWINLEAF in 3D matches the reference" "python3 -I - '$OUT/look' <<'EOF'
import sys
import numpy as np
from PIL import Image
d = sys.argv[1]
g = np.array(Image.open(d + '/frame.png').convert('RGB').resize((240, 160), Image.NEAREST)).astype(int)
r = np.array(Image.open(d + '/ref.png').convert('RGB')).astype(int)
keep = np.ones((160, 240), bool)
keep[40:100, 100:140] = False   # (the player)
keep[0:36, :] = False           # (a new save's achievements note)
diff = np.abs(g - r).sum(2)[keep]
print('    mean %.2f, worst %d' % (diff.mean(), diff.max()))
sys.exit(0 if diff.mean() < 8 and diff.max() < 60 else 1)
EOF"

run wall 'pressuntil A field 5000\nwait 30\nmp warp 40,0,10,13\nwait 200\npress UP 96\nwait 30\nmp where\nquit\n'
check "the house's wall stops the player at (10, 12)" "grep -q 'where map 40.0 xy 10,12' '$OUT/wall/log.txt'"

run walk 'pressuntil A field 5000\nwait 30\nmp warp 40,0,14,14\nwait 200\npress UP 330\nwait 40\nshot route\nmp where\nquit\n'
check "walking north reaches ROUTE 201" "grep -q 'where map 40.1' '$OUT/walk/log.txt'"
check "the 3D pack was loaded" "grep -q '\[s3d\] .*textures' '$OUT/walk/log.txt'"

# the whole matrix: ROUTE 201's east edge leads into SANDGEM TOWN (map 54 of group 40, tools/sinnoh/gen_maps.py)
run east 'pressuntil A field 5000\nwait 30\nmp warp 40,1,52,12\nwait 200\npress RIGHT 220\nwait 60\nmp where\nquit\n'
check "walking east off ROUTE 201 reaches SANDGEM TOWN" "grep -q 'where map 40.54' '$OUT/east/log.txt'"

# the ground's height (BDHC): TWINLEAF's ground is at 16, its pond (rows 27+) at 8, blended between tile centres
run slope 'pressuntil A field 5000\nwait 30\nmp warp 40,0,15,24\nwait 200\nmp s3dground 248 392\nmp s3dground 248 472\nmp s3dground 248 432\nquit\n'
check "the ground: 16 in TWINLEAF, 8 on its pond, 12 between" "grep -q 's3dground 248 392: 16.00' '$OUT/slope/log.txt' && grep -q 's3dground 248 472: 8.00' '$OUT/slope/log.txt' && grep -q 's3dground 248 432: 12.00' '$OUT/slope/log.txt'"

echo "$fail failed (results in $OUT)"
exit $fail
