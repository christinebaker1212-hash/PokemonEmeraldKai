#!/bin/bash
# N clients (2-4) through a local relay server. usage: runN.sh N DIR [DELAY_MS [JITTER_MS]]
#   DIR/a.txt = the host ("Test Room", ALICE); b.txt, c.txt, d.txt = joiners (BOB, CARL, DANA).
#   Logs a.log..d.log, shots in DIR (give each client's shots their own names).
#   Env like run2.sh: WARP (host) / WARP_B / WARP_C / WARP_D (default WARP), MOEMON_DEBUG_PARTY, FPS (default 120),
#   TMO (seconds, default 150), ENV_A..ENV_D ("VAR=value ..." extra environment for one client),
#   WRAP_A..WRAP_D (a command to run that client under, e.g. "gdb -batch -x cmds --args").
. "$(dirname "$0")/common.sh"
N=${1:?usage: runN.sh N DIR [DELAY_MS [JITTER_MS]]}; shift
D="$(cd "${1:?usage: runN.sh N DIR [DELAY_MS [JITTER_MS]]}" && pwd)"; DELAY=${2:-0}; JIT=${3:-0}
build_server || exit 1
rm -f "$D"/*.bmp "$D"/*.png "$D"/[abcd].sav "$D"/[abcd].ini
PORT=$(free_port)
"$SERVER" --port "$PORT" --no-router > "$D/server.log" 2>&1 &
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
NAMES=(ALICE BOB CARL DANA)
PIDS=()
for i in $(seq 0 $((N - 1))); do
    c=$(printf "\\x$(printf %x $((97 + i)))")
    C=${c^^}
    envv="ENV_$C"; warpv="WARP_$C"; wrapv="WRAP_$C"
    if [ "$i" = 0 ]; then auto="host:Test Room"; w=${WARP:-}; else auto="join:Test Room"; w=${!warpv:-$WARP}; fi
    env ${!envv} MOEMON_DEBUG_WARP=$w MOEMON_TEST="$D/$c.txt" MOEMON_COOP_CONFIG="$D/$c.ini" MOEMON_SAVE="$D/$c.sav" \
        MOEMON_DEBUG_NAME=${NAMES[$i]} MOEMON_DEBUG_GENDER=$([ $i = 0 ] && echo F || echo M) MOEMON_COOP_AUTO="$auto" \
        timeout -k 10 "${TMO:-150}" ${!wrapv} "$BIN" > "$D/$c.log" 2>&1 &
    PIDS+=($!)
    sleep 1
done
wait "${PIDS[@]}"
kill $SP ${PP:-} 2>/dev/null
wait 2>/dev/null
grep -h "TIMEOUT\|Segmentation\|Aborted" "$D"/[abcd].log
convert_shots "$D"
echo done
