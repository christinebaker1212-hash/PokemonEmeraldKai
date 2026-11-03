#!/bin/bash
# The dev channel ended in 1.80.0 (BUILDING_COOP.md "Dev update channel (ended)"): every build follows main whatever
# AllowSmallTweaks says. A dev install moves itself to main (the last dev release carries this code), keeps the save and
# the setting; main never takes a dev manifest. Real packages, verified installs and relaunches.
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
unzip -q "$D/dev/MoemonEmeraldKaiCoop-linux.zip" -d "$D/player"
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
run main 1 dev-to-main
grep -q 'installed version' "$D/dev-to-main.log"
grep -q '(main)' "$D/dev-to-main.log"
echo 'ok a dev install moves to main despite AllowSmallTweaks = 1; save and setting preserved'
run main 1 stay-main-tweaks
grep -q 'up to date' "$D/stay-main-tweaks.log"
! grep -q 'installed version' "$D/stay-main-tweaks.log"
echo 'ok AllowSmallTweaks = 1 stays on main'
run main 0 stay-main
grep -q 'up to date' "$D/stay-main.log"
! grep -q 'installed version' "$D/stay-main.log"
echo 'ok current main does not download repeatedly'
run dev 1 refuse-dev
grep -q 'refusing manifest for wrong channel' "$D/refuse-dev.log"
! grep -q 'installed version' "$D/refuse-dev.log"
echo "ok a dev manifest is refused; results in $D"
