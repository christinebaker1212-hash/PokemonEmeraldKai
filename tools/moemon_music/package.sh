#!/bin/bash
# Moemon Music Studio's release packages (.github/workflows/mms-release.yml), from the builds already made:
#   make linux && make windows SDL2_DIR=...   then   ./package.sh DIST
# DIST/MoemonMusicStudio-windows.zip   the exe, SDL2.dll, libwinpthread-1.dll, README, the emulators' licenses
# DIST/MoemonMusicStudio-linux.tar.gz  the program (needs the system's SDL2: libsdl2-2.0-0), README, licenses
set -eu
cd "$(dirname "$0")"
DIST=$(realpath -m "${1:?usage: package.sh DIST}")
mkdir -p "$DIST"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT

# what both carry: the manual and every bundled emulator's license
common() {
    local d=$1
    cp README.md "$d/README.md"
    mkdir -p "$d/licenses"
    cp third_party/README.md "$d/licenses/THIRD_PARTY.md"
    cp third_party/gme/license.txt "$d/licenses/game_music_emu.txt"
    cp third_party/desmume_2sf/IN_XSF_LICENSE "$d/licenses/desmume_2sf.txt"
    cp third_party/he/Readme.txt "$d/licenses/highly_experimental.txt"
    cp third_party/vbam_gsf/License.txt "$d/licenses/vbam_gsf.txt"
    cp third_party/lazyusf2/README.md "$d/licenses/lazyusf2.md"
    cp third_party/psflib/LICENSE "$d/licenses/psflib.txt"
    cp third_party/zlib/LICENSE "$d/licenses/zlib.txt"
}

W="$STAGE/MoemonMusicStudio"
mkdir -p "$W"
cp build/windows/MoemonMusicStudio.exe build/windows/SDL2.dll "$W/"
[ -f build/windows/libwinpthread-1.dll ] && cp build/windows/libwinpthread-1.dll "$W/"
common "$W"
rm -f "$DIST/MoemonMusicStudio-windows.zip"
(cd "$STAGE" && zip -qr9 "$DIST/MoemonMusicStudio-windows.zip" MoemonMusicStudio)

rm -rf "$W"
mkdir -p "$W"
cp build/linux/moemon_music_studio "$W/"
strip "$W/moemon_music_studio" 2>/dev/null || true
common "$W"
tar -C "$STAGE" -czf "$DIST/MoemonMusicStudio-linux.tar.gz" MoemonMusicStudio

ls -l "$DIST"/MoemonMusicStudio-*
