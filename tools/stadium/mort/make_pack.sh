#!/bin/bash
# POKeMON STADIUM's announcer voices (stadium/announcer.s1): the speech archive of Pokemon Stadium (USA) 1.0, which
# the game decodes itself (mort/); the Japanese ones (stadium/announcer_jp.s1) from POKEMON STADIUM 2 (Japan).
# Nothing of the ROM or the voices is in the repository (stadium/ is ignored); the release builds the packs and ships
# them beside the game.
# usage: tools/stadium/mort/make_pack.sh ROM                (a .z64 / .v64 / .n64, or a zip holding one; either game)
#        STADIUM_ROM_URL=URL tools/stadium/mort/make_pack.sh  (the release: downloads it first; the link is a secret)
set -e
REPO="$(cd "$(dirname "$0")/../../.." && pwd)"
WORK="${STADIUM_WORK:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/stadium_voice}"
mkdir -p "$WORK"
ROM="$1"
if [ -z "$ROM" ]; then
    [ -n "$STADIUM_ROM_URL" ] || { echo "usage: $0 ROM  (or STADIUM_ROM_URL=...)"; exit 1; }
    ROM="$WORK/rom.download"
    if [ ! -s "$ROM" ]; then
        URL="$STADIUM_ROM_URL"
        # a Google Drive share link: its direct download
        if [[ "$URL" =~ drive\.google\.com/file/d/([^/]+) ]]; then
            URL="https://drive.usercontent.google.com/download?id=${BASH_REMATCH[1]}&export=download&confirm=t"
        fi
        curl -sSfL --retry 4 -o "$ROM" "$URL"
    fi
fi
python3 -I "$REPO/tools/stadium/mort/make_voice_pack.py" "$ROM"   # (stadium/announcer.s1 or announcer_jp.s1, by the game)
