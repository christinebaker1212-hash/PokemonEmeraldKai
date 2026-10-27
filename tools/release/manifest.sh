#!/bin/bash
# Writes OUTDIR/update.txt for the zips package.sh made there. usage: tools/release/manifest.sh OUTDIR
# The game reads it from the latest release: version, the release's tag, and per platform the zip's
# name, size, SHA-256 and the exe to start after installing; the card art pack if OUTDIR has it.
set -e
cd "$(dirname "$0")/../.."
OUT=${1:?usage: manifest.sh OUTDIR}
. tools/release/channel.sh
{
    echo "# Moemon Emerald Kai Co-op update information (src/platform/updater.c reads this)"
    echo "version $VER"
    echo "tag $TAG"
    echo "channel $CHANNEL"
    for p in windows linux; do
        z="$OUT/MoemonEmeraldKaiCoop-$p.zip"
        [ -f "$z" ] || continue
        exe=MoemonEmeraldKaiCoop; [ $p = windows ] && exe=MoemonEmeraldKaiCoop.exe
        echo "$p $(basename "$z") $(stat -c %s "$z") $(sha256sum "$z" | cut -d' ' -f1) $exe"
    done
    # the printed card pictures (tools/release/art_pack.sh), downloaded once from release ART_TAG (default: this one)
    for z in "$OUT"/moemon-tcg-art-*.zip; do
        [ -f "$z" ] || continue
        v=$(basename "$z" .zip); v=${v#moemon-tcg-art-}
        echo "art $v ${ART_TAG:-$TAG} $(basename "$z") $(stat -c %s "$z") $(sha256sum "$z" | cut -d' ' -f1)"
    done
} > "$OUT/update.txt"
cat "$OUT/update.txt"
