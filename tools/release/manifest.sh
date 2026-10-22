#!/bin/bash
# Writes OUTDIR/update.txt for the zips package.sh made there. usage: tools/release/manifest.sh OUTDIR
# The game reads it from the latest release: version, the release's tag, and per platform the zip's
# name, size, SHA-256 and the exe to start after installing.
set -e
cd "$(dirname "$0")/../.."
OUT=${1:?usage: manifest.sh OUTDIR}
VER=$(sed -n 's/^#define MOEMON_VERSION "\(.*\)"/\1/p' include/mp_version.h)
{
    echo "# Moemon Emerald Kai Co-op update information (src/platform/updater.c reads this)"
    echo "version $VER"
    echo "tag v$VER"
    for p in windows linux; do
        z="$OUT/MoemonEmeraldKaiCoop-$p.zip"
        [ -f "$z" ] || continue
        exe=MoemonEmeraldKaiCoop; [ $p = windows ] && exe=MoemonEmeraldKaiCoop.exe
        echo "$p $(basename "$z") $(stat -c %s "$z") $(sha256sum "$z" | cut -d' ' -f1) $exe"
    done
} > "$OUT/update.txt"
cat "$OUT/update.txt"
