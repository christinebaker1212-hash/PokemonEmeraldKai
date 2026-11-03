#!/bin/bash
# Moemon Maker's published pack (src/platform/updater.c MakerPackWorker, src/maker/maker_overrides.c Maker_ReadDir): a
# release folder with no working copy downloads MoemonMakerPack.zip (tools/maker/build_pack.sh) from a local server,
# installs it into MoemonMakerPack/ and plays its edits; started again with the same pack, nothing is downloaded.
# usage: tools/test/maker_pack.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_pack_test
rm -rf "$T"; mkdir -p "$T/game" "$T/srv" "$T/src/base"
fail=0
cp "$BIN" "$T/game/pokeemerald64"
echo "test" > "$T/game/moemon_release.txt"
cat > "$T/src/base/MAP_LITTLEROOT_TOWN.txt" <<'X'
map MAP_LITTLEROOT_TOWN 20 20
block 9 10 0x3001 0x3004
X
"$ROOT/tools/maker/build_pack.sh" "$T/src" "$T/srv" abc123def456 > /dev/null
PORT=$((20000 + RANDOM % 20000))
(cd "$T/srv" && python3 -m http.server $PORT --bind 127.0.0.1 > "$T/srv.log" 2>&1) &
SRV=$!
trap 'kill $SRV 2>/dev/null' EXIT
sleep 1
printf 'wait 200\npressuntil A field 5000\nwait 60\nquit\n' > "$T/s.txt"
for run in 1 2; do
    (cd "$T/game" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$T/s.txt" MOEMON_SHOTDIR="$T" \
        MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/c.ini" MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_WARP=0,9,10,13 \
        MOEMON_SAVE="$T/p.sav" MOEMON_UPDATE_URL="http://127.0.0.1:$PORT/" MOEMON_MAKER_PACK_URL="http://127.0.0.1:$PORT/" \
        timeout 150 ./pokeemerald64 > "$T/run$run.log" 2>&1)
done
grep -q "maker pack: abc123def456 installed" "$T/run1.log" || { echo "FAIL: the pack wasn't installed"; fail=1; }
[ -f "$T/game/MoemonMakerPack/base/MAP_LITTLEROOT_TOWN.txt" ] || { echo "FAIL: no pack files"; fail=1; }
grep -q "\[maker\] 1 edited maps (.*MoemonMakerPack/base)" "$T/run1.log" "$T/run2.log" || { echo "FAIL: the game didn't read the pack"; fail=1; }
grep -q "maker pack: abc123def456, up to date" "$T/run2.log" || { echo "FAIL: the second start downloaded again"; fail=1; }
[ $fail = 0 ] && echo "maker_pack: OK"
exit $fail
