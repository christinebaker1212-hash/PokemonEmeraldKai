#!/bin/bash
# ORRE's packs as their own zip (docs/ORRE_DECISIONS.md): the game downloads it the first time a player goes to ORRE,
# and again when the packs change. usage: package_orre.sh OUTDIR [ORRE_DIR]   (or package_orre.sh --id)
#   Writes OUTDIR/MoemonOrre.zip (one top folder, orre/) and OUTDIR/orre_id.txt.
#   The id comes from the tools that rip the packs (tools/orre/): the disc is always the same one (make_pack.sh checks
#   it), so the same tools make the same packs, and a release whose tools are the last one's needs no disc at all.
set -e
cd "$(dirname "$0")/../.."
ID=o$(find tools/orre -type f \( -name '*.py' -o -name '*.sh' \) -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum | cut -c1-24)
if [ "$1" = "--id" ]; then echo "$ID"; exit 0; fi
OUT=$(mkdir -p "${1:?usage: package_orre.sh OUTDIR [ORRE_DIR]}" && cd "$1" && pwd)
SRC=$(cd "${2:-orre}" && pwd)
[ -f "$SRC/orre3d.pak" ] && [ -f "$SRC/orre_data.pak" ] && [ -f "$SRC/orre_text.pak" ] \
    || { echo "no ORRE packs in $SRC (tools/orre/make_pack.sh)"; exit 1; }
STAGE=$(mktemp -d)
mkdir -p "$STAGE/orre"
cp "$SRC/orre3d.pak" "$SRC/orre_data.pak" "$SRC/orre_text.pak" "$STAGE/orre/"
rm -f "$OUT/MoemonOrre.zip"
(cd "$STAGE" && zip -9 -qrX "$OUT/MoemonOrre.zip" orre)
rm -rf "$STAGE"
echo "$ID" > "$OUT/orre_id.txt"
echo "$OUT/MoemonOrre.zip ($ID, $(stat -c %s "$OUT/MoemonOrre.zip") bytes)"
