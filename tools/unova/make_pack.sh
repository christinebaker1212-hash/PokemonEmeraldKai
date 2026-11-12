#!/bin/bash
# UNOVA's packs (docs/UNOVA_HANDOFF.md), made from the user's Pokemon Black ROM into unova/: black3d.pak (every map),
# black_text.pak (Black's words), black_scripts.pak (its scripts, zones, events, trainers, wild POKéMON, trainers' words)
# and black_gfx.pak (its people, trainers' battle pictures and backs). The game reads only these: no ROM. Nothing of the
# ROM or the packs is in the repository (unova/ is ignored); the release builds them (the UNOVA_ROM_URL secret) and
# ships them in the game's own zips. Without them the game doesn't offer UNOVA (u3d.c U3D_Available).
# usage: tools/unova/make_pack.sh ROM          (a .nds, or a .zip holding one)
#        UNOVA_ROM_URL=URL tools/unova/make_pack.sh   (the release: downloads the ROM first)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
WORK="${UNOVA_WORK:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/unova}"
mkdir -p "$WORK"
ROM="$1"
if [ -z "$ROM" ]; then
    [ -n "$UNOVA_ROM_URL" ] || { echo "usage: $0 ROM  (or UNOVA_ROM_URL=...)"; exit 1; }
    ROM="$WORK/rom.download"
    if [ ! -s "$ROM" ]; then
        URLS=("$UNOVA_ROM_URL")
        # a Google Drive share link: its direct download, and Drive's older address for it (tried in turn)
        if [[ "$UNOVA_ROM_URL" =~ drive\.google\.com/file/d/([^/]+) ]]; then
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
# the ROM the tools read: Pokemon Black (USA, Europe) (NDSi Enhanced)
echo "26ad0b9967aa279c4a266ee69f52b9b2332399a5  $ROM" | sha1sum -c --quiet \
    || { echo "this isn't Pokemon Black (USA, Europe)"; exit 1; }
# aletheia (Project-Holon's byte-matching decomp: its tools unpack the ROM), pinned
ALETHEIA_COMMIT=ce8fb59a0e4e7e8c445c5696c94a5611189f2007
A="$WORK/aletheia"
if [ "$(git -C "$A" rev-parse HEAD 2>/dev/null)" != "$ALETHEIA_COMMIT" ]; then
    rm -rf "$A" && mkdir -p "$A"
    git -C "$A" init -q
    git -C "$A" remote add origin https://github.com/Project-Holon/aletheia
    git -C "$A" fetch -q --depth 1 origin "$ALETHEIA_COMMIT"
    git -C "$A" checkout -q FETCH_HEAD
fi
rm -rf "$WORK/extract" "$WORK/ov021.bin" "$WORK/ov010.bin"
python3 -I "$A/tools/ndsrom.py" extract "$ROM" -o "$WORK/extract" > /dev/null
python3 -I "$A/tools/blz.py" decompress "$WORK/extract/arm9.bin" -o "$WORK/arm9_dec.bin" > /dev/null
export UNOVA_EXTRACT="$WORK/extract"
cd "$REPO"
mkdir -p unova
python3 -I tools/unova/build_pack.py unova/black3d.pak
python3 -I tools/unova/build_text.py unova/black_text.pak
python3 -I tools/unova/build_scripts.py unova/black_scripts.pak
python3 -I tools/unova/build_gfx.py unova/black_gfx.pak
ls -la unova/
