#!/bin/bash
# SINNOH's pieces the decomp doesn't hold, from the user's Platinum ROM (docs/SINNOH_HANDOFF.md "The user's Platinum
# ROM"): CANALAVE LIBRARY's TV (sinnoh/platinum_libtv.bin, tools/sinnoh/build_library_tv.py) and the BATTLE FRONTIER's
# Scratch-Off Cards (sinnoh/platinum_scratch.bin, tools/sinnoh/build_scratch.py), and OPTIONS > DISPLAY > UI STYLE's
# PLATINUM (sinnoh/ui_platinum.pak: tools/ui_styles/build_pack.py, from the ROM's fonts and battle backgrounds and
# pret/pokeplatinum's interface art, tools/ui_styles/fetch_platinum.sh). Nothing of the ROM or
# what's built from it is in the repository (sinnoh/ is ignored); the release builds it from the PLATINUM_ROM_URL secret
# and ships it with SINNOH's packs. Without it the game holds the screen dark where the TV would be, and the cards' screen
# says what it lacks.
# usage: tools/sinnoh/rom_pack.sh ROM                    (a .nds, or a zip holding one)
#        PLATINUM_ROM_URL=URL tools/sinnoh/rom_pack.sh   (the release: downloads the ROM first)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
WORK="${PLATINUM_WORK:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/platinum_rom}"
mkdir -p "$WORK"
ROM="$1"
if [ -z "$ROM" ]; then
    [ -n "$PLATINUM_ROM_URL" ] || { echo "usage: $0 ROM  (or PLATINUM_ROM_URL=...)"; exit 1; }
    ROM="$WORK/rom.download"
    if [ ! -s "$ROM" ]; then
        URLS=("$PLATINUM_ROM_URL")
        # a Google Drive share link: its direct download, and Drive's older address for it (tried in turn)
        if [[ "$PLATINUM_ROM_URL" =~ drive\.google\.com/file/d/([^/]+) ]]; then
            URLS=("https://drive.usercontent.google.com/download?id=${BASH_REMATCH[1]}&export=download&confirm=t"
                  "https://drive.google.com/uc?export=download&id=${BASH_REMATCH[1]}&confirm=t")
        fi
        ok=
        for try in 1 2 3 4 5 6; do
            URL="${URLS[$(( (try - 1) % ${#URLS[@]} ))]}"
            if curl -sSfL --retry 2 --retry-all-errors --retry-delay 10 --connect-timeout 30 -o "$ROM.part" "$URL" \
               && [ "$(stat -c %s "$ROM.part")" -gt 1000000 ] && ! head -c 512 "$ROM.part" | grep -qi '<html'; then
                mv "$ROM.part" "$ROM"; ok=1; break
            fi
            echo "the ROM's download failed (try $try): waiting $((try * 20)) s"
            rm -f "$ROM.part"
            sleep $((try * 20))
        done
        [ -n "$ok" ] || { echo "the ROM couldn't be downloaded"; exit 1; }
    fi
fi
# a zip holding the ROM
if [ "$(head -c 2 "$ROM")" = "PK" ]; then
    rm -rf "$WORK/romzip" && mkdir -p "$WORK/romzip"
    python3 -I -c "import zipfile,sys; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "$ROM" "$WORK/romzip"
    ROM="$(find "$WORK/romzip" -type f -iname '*.nds' | head -1)"
    [ -n "$ROM" ] || { echo "no .nds in the zip"; exit 1; }
fi
mkdir -p "$REPO/sinnoh"
python3 -I "$REPO/tools/sinnoh/build_library_tv.py" "$ROM" "$REPO/sinnoh/platinum_libtv.bin"
python3 -I "$REPO/tools/sinnoh/build_scratch.py" "$ROM" "$REPO/sinnoh/platinum_scratch.bin"
"$REPO/tools/ui_styles/fetch_platinum.sh" "$WORK/pokeplatinum_ui"
python3 -I "$REPO/tools/ui_styles/build_pack.py" "platinum=$WORK/pokeplatinum_ui:$ROM" "$REPO/sinnoh/ui_platinum.pak"
python3 -I "$REPO/tools/moemon_ballfx/build_ds_pack.py" platinum "$ROM" "$REPO/sinnoh/ballfx_platinum.pak"   # (the ball effects' pieces from Platinum's move animations and ball seals)
