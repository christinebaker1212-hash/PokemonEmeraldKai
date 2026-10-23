#!/bin/bash
# One client. usage: run1.sh DIR [VAR=value...]
#   DIR/a.txt is the harness script (see src/platform/sdl2.c); results (log.txt, shots) land in DIR.
#   Env: TMO (seconds, default 120), KEEPINI=1 keeps DIR/a.ini, BIN (the game).
#   A DIR/a.sav.coop already there is kept (seed save data, e.g. "ball:last = 4").
. "$(dirname "$0")/common.sh"
D="$(cd "${1:?usage: run1.sh DIR [VAR=value...]}" && pwd)"; shift
rm -f "$D"/*.bmp "$D"/*.png "$D/a.sav"; [ -n "$KEEPINI" ] || rm -f "$D/a.ini"
cd "$REPO" || exit 1
env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$D/a.txt" MOEMON_SHOTDIR="$D" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$D/a.ini" MOEMON_SAVE="$D/a.sav" "$@" \
    timeout -k 10 "${TMO:-120}" "$BIN" > "$D/log.txt" 2>&1
rc=$?
convert_shots "$D"
echo "exit $rc"
