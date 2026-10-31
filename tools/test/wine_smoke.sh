#!/bin/bash
# The Windows build under Wine. usage: wine_smoke.sh PKG_DIR [DELAY_MS JIT_MS]
#   PKG_DIR = the unzipped package (MoemonEmeraldKaiCoop.exe, SDL2.dll, server/moemon_server.exe).
#   1) solo: the Windows exe boots, reaches the field and a wild battle's action menu
#   2) mixed: the Windows relay server + a Windows host + a Linux joiner (pokeemerald64) in a
#      co-op wild battle through lagproxy (default 250 ms + 80 ms jitter)
#   3) reverse: Linux server + Linux host, Windows joiner
#   4) panel: the SPECTATE panel between a Linux host and a Windows friend (pixel-for-pixel checksums)
#   Needs wine (apt install wine64). Results in $OUT/wine_smoke. Exit code = failures.
. "$(dirname "$0")/common.sh"
PKG="$(cd "${1:?usage: wine_smoke.sh PKG_DIR [DELAY_MS JIT_MS]}" && pwd)"; DELAY=${2:-250}; JIT=${3:-80}
command -v wine >/dev/null || { echo "wine not installed"; exit 99; }
export WINEDEBUG=-all WINEPREFIX="${WINEPREFIX:-$OUT/wineprefix}"
D="$OUT/wine_smoke"; rm -rf "$D"; mkdir -p "$D/solo" "$D/mixed"; fail=0
GAME="$D/game"; cp -r "$PKG" "$GAME"   # the game writes its save and crash_log.txt next to the exe

# (then a GEN 9 one: its cry is past 511 in the cry tables, the part of them 1.61.4 read wrong)
printf 'pressuntil A field 5000\nshot w0\nmp wild 288:3\npressuntil A action 3000\nshot w1\npress RIGHT 2\nwait 4\npress DOWN 2\nwait 4\npress A 2\npressuntil B field 1500\nwait 10\nmp wild 856:5\npressuntil A action 3000\nshot w2\nquit\n' > "$D/solo/a.txt"
(cd "$GAME" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="Z:$D/solo/a.txt" MOEMON_SHOTDIR="Z:$D/solo" \
    MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="Z:$D/solo/a.ini" MOEMON_SAVE="Z:$D/solo/a.sav" \
    timeout 300 wine MoemonEmeraldKaiCoop.exe > "$D/solo/a.log" 2>&1)
if [ "$(grep -c "action menu at frame" "$D/solo/a.log")" -ge 2 ]; then echo "ok   wine solo (a HOENN and a GEN 9 wild battle)"; else echo "FAIL wine solo (see $D/solo/a.log)"; fail=$((fail+1)); fi

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
# 3) the other way round: Linux relay server + Linux host, Windows joiner (the battle partner's screen)
build_server || exit 99
R="$D/reverse"; mkdir -p "$R"; cp "$TOOLS/scenarios/coop_wild/a.txt" "$TOOLS/scenarios/coop_wild/b.txt" "$R/"
PORT=$(free_port); LP=$(free_port)
"$SERVER" --port "$PORT" --no-router > "$R/server.log" 2>&1 & SP=$!
python3 "$REPO/tools/mp_server/lagproxy.py" "$LP" "$PORT" "$DELAY" "$JIT" > "$R/proxy.log" 2>&1 & PP=$!
sleep 1
(cd "$REPO" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$R/a.txt" MOEMON_SHOTDIR="$R" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,16,12,11 MOEMON_COOP_CONFIG="$R/a.ini" MOEMON_SAVE="$R/a.sav" \
    MOEMON_DEBUG_NAME=ALICE MOEMON_DEBUG_GENDER=F MOEMON_COOP_AUTO="host:Test Room" timeout 400 "$BIN" > "$R/a.log" 2>&1) & AP=$!
sleep 2
(cd "$GAME" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="Z:$R/b.txt" MOEMON_SHOTDIR="Z:$R" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,16,14,11 MOEMON_COOP_CONFIG="Z:$R/b.ini" MOEMON_SAVE="Z:$R/b.sav" \
    MOEMON_DEBUG_NAME=BOB MOEMON_COOP_AUTO="join:Test Room" timeout 400 wine MoemonEmeraldKaiCoop.exe > "$R/b.log" 2>&1) & BP=$!
wait $AP $BP
kill $PP $SP 2>/dev/null; wineserver -k 2>/dev/null; wait 2>/dev/null
if grep -q "co-op: battle over, outcome 1" "$R/a.log" && grep -q "co-op: battle over, outcome 1" "$R/b.log"; then
    echo "ok   wine reverse (Linux server + host, Windows joiner, $DELAY ms)"
else
    echo "FAIL wine reverse (see $R/*.log)"; fail=$((fail+1))
fi
# 4) the SPECTATE panel across platforms: Linux host and Windows friend show each other's screen (walking, then a wild
#    battle); every picture each made of the other's must be the one sent, pixel for pixel (check_spectrace.py)
P="$D/panel"; mkdir -p "$P"
printf 'pressuntil A field 5000\nwaitfor online 6000\nmp panel 1 1\nwait 300\npress LEFT 40\npress UP 30\npress RIGHT 40\npress DOWN 30\nwait 200\nmp wilddouble 0\nmp wild 288:3\nwaitfor battle 2000\nwait 900\nquit\n' > "$P/a.txt"
printf 'pressuntil A field 5000\nwaitfor online 6000\nmp panel 1 2\nwait 300\npress RIGHT 40\npress DOWN 30\npress LEFT 40\npress UP 30\nwait 1500\nquit\n' > "$P/b.txt"
PORT=$(free_port); LP=$(free_port)
"$SERVER" --port "$PORT" --no-router > "$P/server.log" 2>&1 & SP=$!
python3 "$REPO/tools/mp_server/lagproxy.py" "$LP" "$PORT" "$DELAY" "$JIT" > "$P/proxy.log" 2>&1 & PP=$!
sleep 1
(cd "$REPO" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$P/a.txt" MOEMON_SHOTDIR="$P" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,10,10,10 MOEMON_COOP_CONFIG="$P/a.ini" MOEMON_SAVE="$P/a.sav" \
    MOEMON_SPECTRACE=1 MOEMON_DEBUG_NAME=ALICE MOEMON_DEBUG_GENDER=F MOEMON_COOP_AUTO="host:Test Room" timeout 400 "$BIN" > "$P/a.log" 2>&1) & AP=$!
sleep 2
(cd "$GAME" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="Z:$P/b.txt" MOEMON_SHOTDIR="Z:$P" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_TEST_FPS=60 MOEMON_LOBBY=127.0.0.1:$LP MOEMON_DEBUG_WARP=0,10,12,10 MOEMON_COOP_CONFIG="Z:$P/b.ini" MOEMON_SAVE="Z:$P/b.sav" \
    MOEMON_SPECTRACE=1 MOEMON_DEBUG_NAME=BOB MOEMON_COOP_AUTO="join:Test Room" timeout 400 wine MoemonEmeraldKaiCoop.exe > "$P/b.log" 2>&1) & BP=$!
wait $AP $BP
kill $PP $SP 2>/dev/null; wineserver -k 2>/dev/null; wait 2>/dev/null
if python3 "$TOOLS/check_spectrace.py" 30 "$P" > "$P/check.txt" 2>&1; then
    echo "ok   wine panel (Linux host, Windows friend, each other's screens, $DELAY ms)"
else
    echo "FAIL wine panel (see $P/check.txt)"; fail=$((fail+1))
fi
[ -f "$GAME/crash_log.txt" ] && { echo "crash_log.txt:"; tail -8 "$GAME/crash_log.txt"; }
exit $fail
