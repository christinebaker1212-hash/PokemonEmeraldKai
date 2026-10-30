#!/bin/bash
# Real main/dev game processes: disk persistence, forced save and autosave guards.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-/tmp/moemon_dev_save_test}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then
    make linux DEV_BUILD=0 -j"$(nproc)"
    make linux DEV_BUILD=1 DEV_REVISION="${DEV_REVISION:-1}" -j"$(nproc)"
fi
BIN=${DEV_BIN:-$REPO/pokeemerald-dev64}
printf 'wait 180\nsaveprobe\nwait 60\nlog save probe completed\nquit\n' > "$OUT/probe.txt"
run() {
    local bin=$1 dir=$2
    mkdir -p "$dir"
    # Tweaks off deliberately: a dev exe must remain read-only even if updates
    # are disabled or the player is offline while returning to main.
    printf 'AllowSmallTweaks = 0\nautosave = 1\n' > "$dir/player.ini"
    (cd "$dir" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 \
        MOEMON_TEST="$OUT/probe.txt" MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 \
        MOEMON_COOP_CONFIG="$dir/player.ini" MOEMON_CONTROLS_CONFIG="$dir/controls.ini" \
        MOEMON_SAVE="$dir/player.sav" timeout -k 5 60 "$bin" > "$dir/log.txt" 2>&1)
    grep -q 'save probe completed' "$dir/log.txt"
}
D=$(mktemp -d "$OUT/case.XXXXXX")
run "$REPO/pokeemerald64" "$D/main"
grep -q 'saveprobe status 1 autosave 1 dev 0' "$D/main/log.txt"
test -s "$D/main/player.sav"
test -s "$D/main/player.sav.coop"
echo 'ok main still saves'
mkdir -p "$D/existing"
cp "$D/main/"player.sav* "$D/existing/"
(cd "$D/existing" && sha256sum player.sav*) > "$D/before.sha"
run "$BIN" "$D/existing"
grep -q 'saveprobe status 255 autosave 0 dev 1' "$D/existing/log.txt"
(cd "$D/existing" && sha256sum -c "$D/before.sha")
# Match both content and membership: no backups, sidecars or temporary saves added.
find "$D/main" -maxdepth 1 -name 'player.sav*' -printf '%f\n' | sort > "$D/main-files"
find "$D/existing" -maxdepth 1 -name 'player.sav*' -printf '%f\n' | sort > "$D/dev-files"
cmp "$D/main-files" "$D/dev-files"
echo 'ok dev preserves existing save and progress sidecars'
run "$BIN" "$D/empty"
test -z "$(find "$D/empty" -name 'player.sav*')"
grep -q 'saveprobe status 255 autosave 0 dev 1' "$D/empty/log.txt"
echo "ok dev creates no save, sidecar or backup; results in $D"
