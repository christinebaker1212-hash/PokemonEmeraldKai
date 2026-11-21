#!/bin/sh
# Moemon VMU pet: the PMD Collab Normal portraits for an edition's PICTURES (gen_icons.py), at a pinned SpriteCollab
# commit, into vmu_pet/.tools/portraits (never committed). The portraits' credits: dist_extras/PMD_CREDITS.txt.
set -e
COMMIT=948c79de39e3fe3067f1fd26294e19d864325cc9
DIR="$(cd "$(dirname "$0")/.." && pwd)/.tools/portraits"
mkdir -p "$DIR"
for n in $(seq 1 151); do
  f=$(printf '%04d' "$n")
  [ -s "$DIR/$f.png" ] && continue
  curl -sfL --retry 4 -o "$DIR/$f.png" "https://raw.githubusercontent.com/PMDCollab/SpriteCollab/$COMMIT/portrait/$f/Normal.png"
done
echo "portraits in $DIR"
