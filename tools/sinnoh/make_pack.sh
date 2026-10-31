#!/bin/bash
# SINNOH's 3D pack, sinnoh/platinum3d.pak (what the game and the release use): fetches pret/pokeplatinum's map data at
# the pinned commit (fetch_decomp.sh) and converts it (build_pack.py). Run it once before playing SINNOH from a build.
# usage: tools/sinnoh/make_pack.sh [DECOMP_DIR]   (default: ~/.cache/moemon/pokeplatinum, reused next time)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${1:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/pokeplatinum}"
"$REPO/tools/sinnoh/fetch_decomp.sh" "$DIR"
mkdir -p "$REPO/sinnoh"
python3 -I "$REPO/tools/sinnoh/build_pack.py" "$DIR" "$REPO/sinnoh/platinum3d.pak" | tail -1
