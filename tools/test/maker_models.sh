#!/bin/bash
# Moemon Maker M5's 3D models in SINNOH (src/maker/maker_models.c, S3D_AddMesh): a .glb cube (tools/maker/make_test_glb.py:
# Blender's own export format, a PNG texture) placed twice in TWINLEAF TOWN by MoemonMaker/models/MAP_SINNOH_TWINLEAF_TOWN.txt
# (one turned and doubled); the game reads it, places both, and the 3D picture shows them (pictures in
# /tmp/maker_models_test). Needs sinnoh/platinum3d.pak (tools/sinnoh/make_pack.sh).
# usage: tools/test/maker_models.sh [BIN]
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_models_test
rm -rf "$T"; mkdir -p "$T/mm/models" "$T/shots"
fail=0
python3 "$ROOT/tools/maker/make_test_glb.py" "$T/mm/models/cube.glb"
printf 'place cube.glb 12 20\nplace cube.glb 14 19 0 45 200\n' > "$T/mm/models/MAP_SINNOH_TWINLEAF_TOWN.txt"
cat > "$T/play.txt" <<'S'
pressuntil A field 5000
wait 30
mp warp 40,0,13,23
waitfor field 600
wait 60
shot models
quit
S
run() { (cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots$1" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_MAKER_DIR="$T/mm$1" MOEMON_SAVE="$T/play$1.sav" timeout 200 "$BIN" > "$T/play$1.log" 2>&1); }
run ""
mkdir -p "$T/mm_none" "$T/shots_none"; run "_none"   # (the same place without models, to compare)
grep "maker model" "$T/play.log" | head
grep -q "maker model: cube.glb read (1 meshes)" "$T/play.log" || { echo "FAIL: the model wasn't read"; fail=1; }
[ "$(grep -c 'maker model: cube.glb placed' "$T/play.log")" = 2 ] || { echo "FAIL: not both placed"; fail=1; }
python3 - "$T/shots/models.bmp" "$T/shots_none/models.bmp" <<'P' || fail=1
import sys
from PIL import Image
a, b = Image.open(sys.argv[1]).convert('RGB'), Image.open(sys.argv[2]).convert('RGB')
diff = sum(1 for p, q in zip(a.getdata(), b.getdata()) if p != q)
red = sum(1 for (r, g, bb), (r2, g2, b2) in zip(a.getdata(), b.getdata()) if (r, g, bb) != (r2, g2, b2) and r > 150 and bb < 120)
print('pixels changed by the models: %d (%d of the cube\'s colors)' % (diff, red))
sys.exit(0 if red > 100 else 1)
P
# the editor: TWINLEAF's inspector has 3D MODELS; a cube placed there goes in the map's models file
E="$T/editor"; mkdir -p "$E/mm/models"
cp "$T/mm/models/cube.glb" "$E/mm/models/"
printf 'AllowSmallTweaks = 1\n' > "$E/on.ini"
cat > "$E/play.txt" <<'S'
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
maker open MAP_SINNOH_TWINLEAF_TOWN
wait 10
maker model place cube.glb 10 10 0 30 150
wait 5
makershot models
wait 3
quit
S
(cd "$E" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$E/play.txt" \
    MOEMON_SHOTDIR="$E" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$E/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$E/mm" timeout 200 "$BIN" > "$E/play.log" 2>&1)
grep -q "^place cube.glb 10 10 0 30 150$" "$E/mm/models/MAP_SINNOH_TWINLEAF_TOWN.txt" 2>/dev/null || { echo "FAIL: the editor didn't save the placement"; fail=1; }
grep -q "\[maker\] shot .*models.bmp ok" "$E/play.log" || { echo "FAIL: no editor picture"; fail=1; }
[ $fail = 0 ] && echo "maker_models: OK"
exit $fail
