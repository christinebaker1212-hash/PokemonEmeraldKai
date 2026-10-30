#!/bin/bash
# The DANCE machine's songs as their own zip (docs/RHYTHM_DECISIONS.md #10): the game downloads it the first time
# someone steps up to the machine, and again when the songs change. usage: package_songs.sh OUTDIR [SONGS_FOLDER]
#   Writes OUTDIR/MoemonDanceSongs.zip (one top folder, stepmania/, stored: the music is compressed already) and
#   OUTDIR/songs_id.txt: an id from every file's name and contents, the same whenever the songs are the same.
set -e
cd "$(dirname "$0")/../.."
OUT=$(mkdir -p "${1:?usage: package_songs.sh OUTDIR [SONGS_FOLDER]}" && cd "$1" && pwd)
SRC=$(cd "${2:-stepmania}" && pwd)
ID=s$(cd "$SRC" && find . -type f ! -name songs_version.txt ! -name '.*' -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum | cut -c1-24)
STAGE=$(mktemp -d)
mkdir -p "$STAGE/stepmania"
(cd "$SRC" && tar cf - --exclude=songs_version.txt -- *) | (cd "$STAGE/stepmania" && tar xf -)
rm -f "$OUT/MoemonDanceSongs.zip"
(cd "$STAGE" && zip -0 -qrX "$OUT/MoemonDanceSongs.zip" stepmania)
rm -rf "$STAGE"
echo "$ID" > "$OUT/songs_id.txt"
echo "$OUT/MoemonDanceSongs.zip ($ID, $(stat -c %s "$OUT/MoemonDanceSongs.zip") bytes)"
