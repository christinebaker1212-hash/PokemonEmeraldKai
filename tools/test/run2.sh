#!/bin/bash
# Two clients through a local relay server. usage: run2.sh DIR [DELAY_MS [JITTER_MS]]
#   DIR/a.txt = the host ("Test Room", ALICE), DIR/b.txt = the joiner (BOB). Logs a.log / b.log, shots in DIR.
#   DELAY_MS > 0 puts lagproxy.py between both clients and the server (each way), like players on the internet.
#   Env: WARP / WARP_B (MOEMON_DEBUG_WARP for host / joiner), MOEMON_DEBUG_PARTY, FPS (default 120; use 60 with
#   a delay so the timing is real), TMO (seconds, default 150), NAME_A / NAME_B,
#   ENV_A / ENV_B ("VAR=value VAR2=value" extra environment for one client, e.g. MOEMON_DEBUG_PARTY=283:46).
. "$(dirname "$0")/common.sh"
D="$(cd "${1:?usage: run2.sh DIR [DELAY_MS [JITTER_MS]]}" && pwd)"; DELAY=${2:-0}; JIT=${3:-0}
build_server || exit 1
rm -f "$D"/*.bmp "$D"/*.png "$D"/barrier_* "$D/a.sav" "$D/b.sav" "$D/a.ini" "$D/b.ini"
PORT=$(free_port)
MOEMON_STORE_DIR="$D" "$SERVER" --port "$PORT" --no-router > "$D/server.log" 2>&1 &
SP=$!
LOBBY=$PORT
if [ "$DELAY" -gt 0 ]; then
    LOBBY=$(free_port)
    python3 "$REPO/tools/mp_server/lagproxy.py" "$LOBBY" "$PORT" "$DELAY" "$JIT" > "$D/proxy.log" 2>&1 &
    PP=$!
fi
sleep 1
cd "$REPO" || exit 1
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_SHOTDIR="$D" MOEMON_LOBBY=127.0.0.1:$LOBBY \
       MOEMON_DEBUG_QUICKSTART=1 MOEMON_TEST_FPS=${FPS:-120}
env $ENV_A MOEMON_DEBUG_WARP=${WARP:-} MOEMON_TEST="$D/a.txt" MOEMON_COOP_CONFIG="$D/a.ini" MOEMON_SAVE="$D/a.sav" \
    MOEMON_DEBUG_NAME=${NAME_A:-ALICE} MOEMON_DEBUG_GENDER=F MOEMON_COOP_AUTO="host:Test Room" \
    timeout -k 10 "${TMO:-150}" "$BIN" > "$D/a.log" 2>&1 &
AP=$!
sleep 1
env $ENV_B MOEMON_DEBUG_WARP=${WARP_B:-$WARP} MOEMON_TEST="$D/b.txt" MOEMON_COOP_CONFIG="$D/b.ini" MOEMON_SAVE="$D/b.sav" \
    MOEMON_DEBUG_NAME=${NAME_B:-BOB} MOEMON_COOP_AUTO="join:Test Room" \
    timeout -k 10 "${TMO:-150}" "$BIN" > "$D/b.log" 2>&1 &
BP=$!
wait $AP $BP
kill $SP ${PP:-} 2>/dev/null
wait 2>/dev/null
grep -h "TIMEOUT\|Segmentation\|Aborted" "$D/a.log" "$D/b.log"
convert_shots "$D"
echo done
