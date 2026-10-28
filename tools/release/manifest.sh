#!/bin/bash
# Writes OUTDIR/update.txt for the zips package.sh made there. usage: tools/release/manifest.sh OUTDIR
# The game reads it from the latest release: version, the release's tag, and per platform the zip's
# name, size, SHA-256 and the exe to start after installing; and where the DANCE machine's songs are.
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
    # the DANCE machine's songs: a line the release workflow made (songs.txt: "songs <tag> <zip> <bytes> <sha256>
    # <id>", the tag of the release that has the zip: the last one whose songs were different)
    if [ -f "$OUT/songs.txt" ]; then cat "$OUT/songs.txt"; fi
} > "$OUT/update.txt"
cat "$OUT/update.txt"
