#!/bin/bash
# Moemon Maker on SINNOH (needs sinnoh/platinum3d.pak: tools/sinnoh/make_pack.sh; skipped without it): TWINLEAF TOWN shown
# by what each tile does (the 3D pack's blocks), a tile blocked in Collision mode and saved, and the INSPECTOR's
# 3D PREVIEW (s3d.c S3D_RenderPreview) through the game's camera. Pictures in /tmp/maker_sinnoh_test.
# usage: tools/test/maker_sinnoh.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_sinnoh_test
[ -f "$ROOT/sinnoh/platinum3d.pak" ] || { echo "maker_sinnoh: skipped (no sinnoh/platinum3d.pak)"; exit 0; }
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_SINNOH_TWINLEAF_TOWN\nwait 4\nmaker stats\n'
    printf 'maker mode 1\nmaker coll 2\nmaker paint 10 10\nmaker mode 0\nmaker pick 9 13\nwait 4\nmakershot twinleaf\nmaker save\nwait 2\nquit\n'
} > "$T/s.txt"
(cd "$ROOT" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/s.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/s.sav" timeout 150 "$BIN" > "$T/run.log" 2>&1)
grep -q "stats MAP_SINNOH_TWINLEAF_TOWN: .*class2=.*class5=4" "$T/run.log" || { echo "FAIL: TWINLEAF's tiles don't read as SINNOH's (its water and 4 doors)"; fail=1; }
grep -q "^block 10 10 0x[0-9A-F]* 0x[0-9A-F]*" "$T/mm/base/MAP_SINNOH_TWINLEAF_TOWN.txt" 2>/dev/null || { echo "FAIL: no collision edit"; fail=1; }
grep -q "shot .*twinleaf.bmp ok" "$T/run.log" || { echo "FAIL: no picture"; fail=1; }
[ $fail = 0 ] && echo "maker_sinnoh: OK"
exit $fail
