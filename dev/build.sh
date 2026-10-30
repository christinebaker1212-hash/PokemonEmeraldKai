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
    # POSIX-threaded MinGW builds also depend on this compiler runtime.
    if x86_64-w64-mingw32-objdump -p "$out/MoemonEmeraldKaiCoop.exe" | grep 'DLL Name: libwinpthread-1.dll' >/dev/null; then
        pthread_dll=$(x86_64-w64-mingw32-gcc -print-file-name=libwinpthread-1.dll)
        [ -f "$pthread_dll" ] || { echo 'Required libwinpthread-1.dll was not found.' >&2; exit 1; }
        cp "$pthread_dll" "$out/"
    fi
    ;;
*) echo 'Usage: dev/build.sh linux|windows [revision]' >&2; exit 1;;
esac
echo "Dev build staged in $out"
