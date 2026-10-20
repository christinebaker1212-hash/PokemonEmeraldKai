#!/bin/bash
# The Windows build under Wine. usage: wine_smoke.sh PKG_DIR [DELAY_MS JIT_MS]
#   PKG_DIR = the unzipped package (MoemonEmeraldKaiCoop.exe, SDL2.dll, server/moemon_server.exe).
#   1) solo: the Windows exe boots, reaches the field and a wild battle's action menu
#   2) mixed: the Windows relay server + a Windows host + a Linux joiner (pokeemerald64) in a
#      co-op wild battle through lagproxy (default 250 ms + 80 ms jitter)
#   Needs wine (apt install wine64). Results in $OUT/wine_smoke. Exit code = failures.
. "$(dirname "$0")/common.sh"
PKG="$(cd "${1:?usage: wine_smoke.sh PKG_DIR [DELAY_MS JIT_MS]}" && pwd)"; DELAY=${2:-250}; JIT=${3:-80}
command -v wine >/dev/null || { echo "wine not installed"; exit 99; }
export WINEDEBUG=-all WINEPREFIX="${WINEPREFIX:-$OUT/wineprefix}"
D="$OUT/wine_smoke"; rm -rf "$D"; mkdir -p "$D/solo" "$D/mixed"; fail=0
GAME="$D/game"; cp -r "$PKG" "$GAME"   # the game writes its save and crash_log.txt next to the exe

printf 'pressuntil A field 5000\nshot w0\nmp wild 288:3\npressuntil A action 3000\nshot w1\nquit\n' > "$D/solo/a.txt"
(cd "$GAME" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="Z:$D/solo/a.txt" MOEMON_SHOTDIR="Z:$D/solo" \
    MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="Z:$D/solo/a.ini" MOEMON_SAVE="Z:$D/solo/a.sav" \
    timeout 300 wine MoemonEmeraldKaiCoop.exe > "$D/solo/a.log" 2>&1)
if grep -q "action menu at frame" "$D/solo/a.log"; then echo "ok   wine solo"; else echo "FAIL wine solo (see $D/solo/a.log)"; fail=$((fail+1)); fi

M="$D/mixed"; cp "$TOOLS/scenarios/coop_wild/a.txt" "$TOOLS/scenarios/coop_wild/b.txt" "$M/"
PORT=$(free_port); LP=$(free_port)
(cd "$GAME/server" && exec wine moemon_server.exe --port "$PORT" --no-router > "$M/server.log" 2>&1) & SP=$!
sleep 3
python3 "$REPO/tools/mp_server/lagproxy.py" "$LP" "$PORT" "$DELAY" "$JIT" > "$M/proxy.log" 2>&1 & PP=$!
sleep 1
(cd "$GAME" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="Z:$M/a.txt" MOEMON_SHOTDIR="Z:$M" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,16,12,11 MOEMON_COOP_CONFIG="Z:$M/a.ini" MOEMON_SAVE="Z:$M/a.sav" \
    MOEMON_DEBUG_NAME=ALICE MOEMON_DEBUG_GENDER=F MOEMON_COOP_AUTO="host:Test Room" timeout 400 wine MoemonEmeraldKaiCoop.exe > "$M/a.log" 2>&1) & AP=$!
sleep 2
(cd "$REPO" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$M/b.txt" MOEMON_SHOTDIR="$M" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,16,14,11 MOEMON_COOP_CONFIG="$M/b.ini" MOEMON_SAVE="$M/b.sav" \
    MOEMON_DEBUG_NAME=BOB MOEMON_COOP_AUTO="join:Test Room" timeout 400 "$BIN" > "$M/b.log" 2>&1) & BP=$!
wait $AP $BP
kill $PP 2>/dev/null; kill $SP 2>/dev/null; wineserver -k 2>/dev/null; wait 2>/dev/null
if grep -q "co-op: battle over, outcome 1" "$M/a.log" && grep -q "co-op: battle over, outcome 1" "$M/b.log"; then
    echo "ok   wine mixed (Windows server + host, Linux joiner, $DELAY ms)"
else
    echo "FAIL wine mixed (see $M/*.log)"; fail=$((fail+1))
fi
[ -f "$GAME/crash_log.txt" ] && { echo "crash_log.txt:"; tail -8 "$GAME/crash_log.txt"; }
exit $fail
