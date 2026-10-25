#!/bin/bash
# REJOIN after joining a friend's own server with an invite code (here its address) and a password:
#   b1: JOIN A GAME > ENTER CODE OR IP, the address, the password -> joined;
#   b2: a new session: the main screen's REJOIN gets back in (no code, no password);
#   b3: a new session: JOIN A GAME > ENTER CODE OR IP starts with the last code (Enter alone joins).
# The "online lobby" is a dead address, so the friend's server is a server of its own, as for a home host.
# usage: rejoin_smoke.sh [OUT]
. "$(dirname "$0")/common.sh"
D="${1:-$OUT/rejoin_smoke}"; rm -rf "$D"; mkdir -p "$D"; D="$(cd "$D" && pwd)"
build_server || exit 1
PORT=$(free_port)
MOEMON_STORE_DIR="$D" "$SERVER" --port "$PORT" --no-router > "$D/server.log" 2>&1 &
SP=$!
sleep 1
cd "$REPO" || exit 1
for f in a b1 b2 b3; do sed "s/@ADDR@/127.0.0.1:$PORT/" "$TOOLS/rejoin_smoke/$f.txt" > "$D/$f.txt"; done
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_SHOTDIR="$D" MOEMON_DEBUG_QUICKSTART=1 MOEMON_TEST_FPS=120
env MOEMON_LOBBY=127.0.0.1:$PORT MOEMON_TEST="$D/a.txt" MOEMON_COOP_CONFIG="$D/a.ini" MOEMON_CONTROLS_CONFIG="$D/a_controls.ini" \
    MOEMON_SAVE="$D/a.sav" MOEMON_DEBUG_NAME=ALICE MOEMON_COOP_AUTO="host:Rejoin Room:secret" timeout -k 10 200 "$BIN" > "$D/a.log" 2>&1 &
AP=$!
sleep 3
fail=0
for b in b1 b2 b3; do
    env MOEMON_LOBBY=127.0.0.1:1 MOEMON_SKIP_COOP_SETUP=0 MOEMON_TEST="$D/$b.txt" MOEMON_COOP_CONFIG="$D/b.ini" \
        MOEMON_CONTROLS_CONFIG="$D/b_controls.ini" MOEMON_SAVE="$D/b.sav" MOEMON_DEBUG_NAME=BOB timeout -k 10 60 "$BIN" > "$D/$b.log" 2>&1
done
grep -q "joined Rejoin Room (at 127.0.0.1:$PORT)" "$D/b1.log" && echo "ok   joined with the address and password" || { echo "FAIL b1: not joined"; fail=1; }
grep -q "^last_join_code = 127.0.0.1:$PORT" "$D/b.ini" && echo "ok   code remembered" || { echo "FAIL: last_join_code not saved"; fail=1; }
grep -q "rejoin: Rejoin Room at 127.0.0.1:$PORT" "$D/b2.log" && grep -q "joined Rejoin Room" "$D/b2.log" \
    && ! grep -q "invite code entered" "$D/b2.log" && echo "ok   REJOIN without the code or password" || { echo "FAIL b2: REJOIN"; fail=1; }
grep -q "invite code entered: \"127.0.0.1:$PORT\"" "$D/b3.log" && grep -q "joined Rejoin Room" "$D/b3.log" \
    && echo "ok   ENTER CODE OR IP starts with the last code" || { echo "FAIL b3: code box"; fail=1; }
kill $AP $SP 2>/dev/null; wait 2>/dev/null
convert_shots "$D"
exit $fail
