#!/bin/bash
# Builds the Windows exe (pokeemerald64.exe) next to a Linux build without mixing their objects:
# both use build/pc64, so the Windows objects live in build/pc64-win between builds.
# usage: tools/release/build_windows.sh SDL2_MINGW_DIR   (the SDL2 mingw development package, e.g. SDL2-2.30.9)
set -e
cd "$(dirname "$0")/../.."
SDL=$(cd "${1:?usage: build_windows.sh SDL2_MINGW_DIR}" && pwd)
shift
[ -e SDL2 ] && [ ! -L SDL2 ] && { echo "./SDL2 exists and isn't a link; move it away first"; exit 1; }
# Dev targets have a separate revision-specific build directory and filename.
if [ "${DEV_BUILD:-0}" = 1 ]; then
    ln -sfn "$SDL" SDL2
    trap 'rm -f SDL2' EXIT
    make winwsl DEV_BUILD=1 DEV_REVISION="${DEV_REVISION:-0}" -j"$(nproc)" "$@"
    exit
fi
[ -d build/pc64-win ] || mkdir -p build/pc64-win
[ -d build/pc64 ] && mv build/pc64 build/pc64-linux
mv build/pc64-win build/pc64
ln -sfn "$SDL" SDL2
rc=0
make winwsl -j"$(nproc)" "$@" || rc=$?
mv build/pc64 build/pc64-win
[ -d build/pc64-linux ] && mv build/pc64-linux build/pc64
rm -f SDL2
exit $rc
