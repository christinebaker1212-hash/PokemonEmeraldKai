#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
root=$PWD
out="$root/dev/builds/linux"
[ -x "$out/MoemonEmeraldKaiCoop" ] || { echo 'Build it with dev/build.sh linux first.' >&2; exit 1; }
if [ ! -e "$out/moemon_coop.ini" ]; then
    printf 'AllowSmallTweaks = 1\nautosave = 0\n' > "$out/moemon_coop.ini"
fi
export MOEMON_COOP_CONFIG="$out/moemon_coop.ini"
export MOEMON_DISPLAY_CONFIG="$out/moemon_display.ini"
export MOEMON_CONTROLS_CONFIG="$out/moemon_controls.ini"
export MOEMON_NO_UPDATE=1
exec "$out/MoemonEmeraldKaiCoop" "$@"
