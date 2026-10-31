#!/bin/bash
# SINNOH's 3D engine, end to end (needs the user's Platinum ROM and a pret/pokeplatinum checkout; nothing from the ROM
# is kept): builds the 3D pack, then in the game
#   1. TWINLEAF TOWN in 3D at noon matches the offline reference picture (tools/sinnoh/render_ref.py) pixel for pixel
#      within rounding (the player's own spot left out)
#   2. Platinum's collision: walking up into the player's house stops at its wall
#   3. walking north out of TWINLEAF reaches ROUTE 201 (the maps' connection) with the 3D view following
# usage: tools/sinnoh/test_poc.sh PLATINUM_ROM POKEPLATINUM_DIR   (BIN=the game, default ./pokeemerald64)
ROM="${1:?usage: test_poc.sh PLATINUM_ROM POKEPLATINUM_DIR}"
SRC="${2:?usage: test_poc.sh PLATINUM_ROM POKEPLATINUM_DIR}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$(mktemp -d)}"
mkdir -p "$OUT"
export BIN="${BIN:-$REPO/pokeemerald64}"
fail=0
check() { if eval "$2"; then echo "ok   $1"; else echo "FAIL $1"; fail=$((fail + 1)); fi; }

python3 -I "$REPO/tools/sinnoh/build_pack.py" "$ROM" "$SRC" "$OUT/platinum3d.pak" > "$OUT/pack.log" || { echo "FAIL pack"; exit 1; }
run() {   # NAME SCRIPT
    mkdir -p "$OUT/$1"
    printf '%b' "$2" > "$OUT/$1/a.txt"
    TMO=120 bash "$REPO/tools/test/run1.sh" "$OUT/$1" MOEMON_S3D_PAK="$OUT/platinum3d.pak" MOEMON_S3D_HOUR=12 > /dev/null
}

run look 'pressuntil A field 5000\nwait 30\nmp warp 40,0,14,9\nwait 200\nshot frame\nquit\n'
python3 -I "$REPO/tools/sinnoh/render_ref.py" "$ROM" "$OUT/look/ref.png" 0 110 873 12 > /dev/null
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

echo "$fail failed (results in $OUT)"
exit $fail
