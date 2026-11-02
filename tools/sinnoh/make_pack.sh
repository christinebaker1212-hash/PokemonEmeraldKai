#!/bin/bash
# SINNOH's 3D pack, sinnoh/platinum3d.pak, its people's sprites and battle pictures, sinnoh/platinum_gfx.pak, and its
# words, sinnoh/platinum_text.pak (Platinum's own message banks: what the game and the release use): fetches
# pret/pokeplatinum's data at the pinned commit (fetch_decomp.sh) and converts it (build_pack.py, build_gfx.py,
# build_text.py). Run it once before playing SINNOH from a build. Nothing of it is in the repository.
# usage: tools/sinnoh/make_pack.sh [DECOMP_DIR]   (default: ~/.cache/moemon/pokeplatinum, reused next time)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${1:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/pokeplatinum}"
"$REPO/tools/sinnoh/fetch_decomp.sh" "$DIR"
mkdir -p "$REPO/sinnoh"
python3 -I "$REPO/tools/sinnoh/build_pack.py" "$DIR" "$REPO/sinnoh/platinum3d.pak" | tail -1
python3 -I "$REPO/tools/sinnoh/build_gfx.py" "$DIR" "$REPO/sinnoh/platinum_gfx.pak"   # (its people as Platinum draws them)
python3 -I "$REPO/tools/sinnoh/build_text.py" "$DIR" "$REPO/sinnoh/platinum_text.pak"
# Platinum's scripts, converted to this game's (the build includes them: sinnoh/platinum_scripts.inc)
echo "$DIR" > "$REPO/sinnoh/decomp_dir"
python3 -I "$REPO/tools/sinnoh/gen_scripts.py" "$DIR" "$REPO/sinnoh/platinum_scripts.inc"
