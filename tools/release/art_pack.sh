#!/bin/bash
# The printed card pictures for a release: tools/release/art_pack.sh OUTDIR
# Downloads the scans (tools/tcg/fetch_art.py), builds the pack (tools/tcg/build_art_pack.py) and checks
# it is the pack this build looks for (TCG_ART_VERSION in include/tcg/art_pack.h, a hash of every
# picture), then puts moemon-tcg-art-<version>.zip in OUTDIR. The game's updater downloads it once
# (update.txt's "art" line, tools/release/manifest.sh). Needs Python 3 with Pillow 12.3.0: another
# Pillow can encode the JPEGs differently, and then the version doesn't match and nothing ships.
set -e
cd "$(dirname "$0")/../.."
OUT=$(mkdir -p "${1:?usage: art_pack.sh OUTDIR}" && cd "$1" && pwd)
WANT=$(sed -n 's/^#define TCG_ART_VERSION "\(.*\)"/\1/p' include/tcg/art_pack.h)
python3 tools/tcg/fetch_art.py
python3 tools/tcg/build_art_pack.py > /dev/null
GOT=$(sed -n 's/^#define TCG_ART_VERSION "\(.*\)"/\1/p' include/tcg/art_pack.h)
# the zip's own checksum isn't part of the version (zip headers differ between Pythons): keep the files as committed
git checkout -- include/tcg/art_pack.h tools/tcg/art_pack.json tools/tcg/art_manifest.json
if [ "$GOT" != "$WANT" ]; then
    echo "art_pack.sh: built pack $GOT, the game looks for $WANT (scans or Pillow changed)" >&2
    exit 1
fi
cp "dev/artwork/tcg-art/moemon-tcg-art-$WANT.zip" "$OUT/"
echo "$OUT/moemon-tcg-art-$WANT.zip"
