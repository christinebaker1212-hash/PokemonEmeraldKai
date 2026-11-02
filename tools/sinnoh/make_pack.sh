#!/bin/bash
# SINNOH's 3D pack, sinnoh/platinum3d.pak, and its people's sprites and battle pictures, sinnoh/platinum_gfx.pak (what
# the game and the release use): fetches pret/pokeplatinum's data at the
# pinned commit (fetch_decomp.sh) and converts it (build_pack.py). Run it once before playing SINNOH from a build.
# SINNOH's words are the game's own, written for it and kept in its scripts (no text is taken from Platinum).
# usage: tools/sinnoh/make_pack.sh [DECOMP_DIR]   (default: ~/.cache/moemon/pokeplatinum, reused next time)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${1:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/pokeplatinum}"
"$REPO/tools/sinnoh/fetch_decomp.sh" "$DIR"
mkdir -p "$REPO/sinnoh"
python3 -I "$REPO/tools/sinnoh/build_pack.py" "$DIR" "$REPO/sinnoh/platinum3d.pak" | tail -1
python3 -I "$REPO/tools/sinnoh/build_gfx.py" "$DIR" "$REPO/sinnoh/platinum_gfx.pak"   # (its people as Platinum draws them)
# the TRAINER CARD's JOHTO card (HeartGold's) and SINNOH card (Platinum's): sinnoh/cards.pak (build_cards.py)
HGSS="${XDG_CACHE_HOME:-$HOME/.cache}/moemon/pokeheartgold_card"
"$REPO/tools/sinnoh/fetch_hgss_card.sh" "$HGSS"
python3 -I "$REPO/tools/sinnoh/build_cards.py" "$DIR" "$HGSS" "$REPO/sinnoh/cards.pak"
