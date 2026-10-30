#!/bin/bash
# Channel changes with actual main/dev packages, verified installs and relaunches.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-/tmp/moemon_dev_update_test}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
if [ -z "${NOBUILD:-}" ]; then
    make linux DEV_BUILD=0 -j"$(nproc)"
    make linux DEV_BUILD=1 DEV_REVISION="${DEV_REVISION:-1}" -j"$(nproc)"
fi
D=$(mktemp -d "$OUT/case.XXXXXX")
CHANNEL=main tools/release/package.sh linux "$D/main" >/dev/null
CHANNEL=main tools/release/manifest.sh "$D/main" >/dev/null
CHANNEL=dev DEV_REVISION=${DEV_REVISION:-1} tools/release/package.sh linux "$D/dev" >/dev/null
CHANNEL=dev DEV_REVISION=${DEV_REVISION:-1} tools/release/manifest.sh "$D/dev" >/dev/null
# Actual server binds an ephemeral loopback port; no fixed-port conflicts.
python3 - "$D" > "$D/port" 2> "$D/http.log" <<'PY' &
import http.server, os, sys
os.chdir(sys.argv[1])
s = http.server.ThreadingHTTPServer(('127.0.0.1', 0), http.server.SimpleHTTPRequestHandler)
print(s.server_port, flush=True)
s.serve_forever()
PY
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT
for _ in {1..50}; do [ ! -s "$D/port" ] || break; sleep .1; done
PORT=$(cat "$D/port")
printf 'wait 30\nlog channel test reached game\nquit\n' > "$D/script.txt"
unzip -q "$D/main/MoemonEmeraldKaiCoop-linux.zip" -d "$D/player"
G="$D/player/MoemonEmeraldKaiCoop"
# An opaque player save must survive updater transitions byte for byte.
head -c 131072 /dev/urandom > "$G/pokeemerald.sav"
sha256sum "$G/pokeemerald.sav" > "$D/save.sha"
run() {
    local channel=$1 on=$2 name=$3
    printf 'AllowSmallTweaks = %s\nAutoUpdate = 1\n' "$on" > "$G/moemon_coop.ini"
    (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$D/script.txt" \
        MOEMON_UPDATE_URL="http://127.0.0.1:$PORT/$channel/" \
        timeout -k 5 60 ./MoemonEmeraldKaiCoop > "$D/$name.log" 2>&1)
    grep -q 'channel test reached game' "$D/$name.log"
    sha256sum -c --quiet "$D/save.sha"
    grep -q "AllowSmallTweaks = $on" "$G/moemon_coop.ini"
}
run dev 1 enter-dev
grep -q 'installed version' "$D/enter-dev.log"
grep -q '(dev)' "$D/enter-dev.log"
echo 'ok main switches to dev and restarts; save and opt-in preserved'
run dev 1 stay-dev
grep -q 'up to date' "$D/stay-dev.log"
! grep -q 'installed version' "$D/stay-dev.log"
echo 'ok current dev does not download repeatedly'
run main 0 return-main
grep -q 'installed version' "$D/return-main.log"
grep -q '(main)' "$D/return-main.log"
echo 'ok dev returns to main despite its lower version'
run main 0 stay-main
grep -q 'up to date' "$D/stay-main.log"
! grep -q 'installed version' "$D/stay-main.log"
echo 'ok current main does not download repeatedly'
run main 1 wrong-channel
grep -q 'refusing manifest for wrong channel' "$D/wrong-channel.log"
! grep -q 'installed version' "$D/wrong-channel.log"
echo 'ok wrong-channel manifest rejected'
sed -i 's/^version .*/version 1.0.0.1/' "$D/dev/update.txt"
run dev 1 stale-dev
grep -q 'dev update is based on an older main version' "$D/stale-dev.log"
! grep -q 'installed version' "$D/stale-dev.log"
echo "ok stale dev content rejected; results in $D"
