#!/bin/bash
# A relay server one co-op version ahead: the game must report the mismatch instead of hanging.
# usage: version_check.sh   (results in $OUT/version_check)
. "$(dirname "$0")/common.sh"
d="$OUT/version_check"; rm -rf "$d"; mkdir -p "$d"
ver=$(sed -n 's/^#define MP_PROTOCOL_VERSION \([0-9]*\).*/\1/p' "$REPO/src/mp/mp_net.h")
srv="$TOOLS/.bin/moemon_server_next"
mkdir -p "$TOOLS/.bin"
gcc -O2 -pthread -DMP_PROTOCOL_VERSION=$((ver + 1)) -o "$srv" "$REPO/tools/mp_server/main.c" "$REPO/src/mp/mp_net.c" \
    "$REPO/src/mp/mp_server.c" "$REPO/src/mp/mp_portmap.c" "$REPO/src/mp/mp_store.c" || exit 1
port=$(free_port)
"$srv" --port "$port" --no-router > "$d/server.log" 2>&1 & sp=$!
sleep 1
printf 'pressuntil A field 5000\nwait 200\nquit\n' > "$d/a.txt"
TMO=60 "$TOOLS/run1.sh" "$d" MOEMON_LOBBY=127.0.0.1:$port "MOEMON_COOP_AUTO=host:Test Room" > /dev/null
kill $sp 2>/dev/null
if grep -q "version mismatch: ours $ver, theirs $((ver + 1))" "$d/log.txt"; then echo "ok   version_check"; else echo "FAIL version_check (see $d/log.txt)"; exit 1; fi
