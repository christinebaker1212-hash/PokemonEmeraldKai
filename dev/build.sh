#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
platform=${1:-linux}
revision=${2:-1}
[[ $revision =~ ^[0-9]+$ ]] || { echo 'Revision must be numeric.' >&2; exit 1; }
case "$platform" in
linux)
    make linux DEV_BUILD=1 DEV_REVISION="$revision" -j"$(nproc)"
    out=dev/builds/linux
    mkdir -p "$out"
    cp pokeemerald-dev64 "$out/MoemonEmeraldKaiCoop"
    ;;
windows)
    DEV_BUILD=1 DEV_REVISION="$revision" tools/release/build_windows.sh "${SDL2_DIR:?Set SDL2_DIR to the checked MinGW SDL2 package}"
    out=dev/builds/windows
    mkdir -p "$out"
    cp pokeemerald-dev64.exe "$out/MoemonEmeraldKaiCoop.exe"
    cp "$SDL2_DIR/x86_64-w64-mingw32/bin/SDL2.dll" "$out/"
    ;;
*) echo 'Usage: dev/build.sh linux|windows [revision]' >&2; exit 1;;
esac
mkdir -p "$out/tcg"
cp -R graphics/tcg/original "$out/tcg/"
echo "Dev build staged in $out"
