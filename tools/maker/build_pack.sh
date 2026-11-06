#!/bin/bash
# Moemon Maker: the published pack from the repository's MoemonMaker/ (the user's edits and music), as the game downloads
# it (src/platform/updater.c MakerPackWorker): OUT/MoemonMakerPack.zip (base/*.txt, music/*.kaisong, lua/*.lua, tilesets/*.txt, pack_version.txt)
# and OUT/maker-pack.txt ("makerpack TAG ZIP BYTES SHA256 ID"). Run by .github/workflows/maker-pack.yml.
# usage: tools/maker/build_pack.sh MOEMONMAKER_DIR OUT [ID] [TAG]
# ID: the pack's id; by default a hash of its files, so the same pack is the same id whether it came in a release's zip
# or from the maker-pack release (the game downloads only a pack it doesn't have)
set -eu
SRC=$1 OUT=$2 ID=${3:-} TAG=${4:-maker-pack}
rm -rf "$OUT/pack"; mkdir -p "$OUT/pack/base" "$OUT/pack/music" "$OUT/pack/lua" "$OUT/pack/tilesets"
cp "$SRC"/base/*.txt "$OUT/pack/base/" 2>/dev/null || true
cp "$SRC"/music/*.kaisong "$OUT/pack/music/" 2>/dev/null || true
cp "$SRC"/lua/*.lua "$OUT/pack/lua/" 2>/dev/null || true   # (M5's Lua hooks)
cp "$SRC"/tilesets/*.txt "$OUT/pack/tilesets/" 2>/dev/null || true   # (M5's tileset edits)
if [ -z "$ID" ]; then
    ID=$( (cd "$OUT/pack" && find . -type f | LC_ALL=C sort | xargs -r sha256sum) | sha256sum | cut -c1-40)
fi
echo "$ID" > "$OUT/pack/pack_version.txt"
rm -f "$OUT/MoemonMakerPack.zip"
(cd "$OUT/pack" && zip -qrX ../MoemonMakerPack.zip .)
echo "makerpack $TAG MoemonMakerPack.zip $(stat -c %s "$OUT/MoemonMakerPack.zip") $(sha256sum "$OUT/MoemonMakerPack.zip" | cut -d' ' -f1) $ID" > "$OUT/maker-pack.txt"
cat "$OUT/maker-pack.txt"
