#!/bin/bash
# Fetches Card-Forge/forge at the pinned commit (tools/mtg/forge_commit.txt): only what the C translation reads, the
# card scripts (forge-gui/res/cardsfolder) and the Java it is translated from (forge-game, forge-ai, forge-core).
# usage: tools/mtg/fetch_forge.sh [DIR]   (default: ~/.cache/moemon/forge, reused next time)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${1:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/forge}"
COMMIT="$(cat "$REPO/tools/mtg/forge_commit.txt")"
if [ ! -d "$DIR/.git" ]; then
    git clone -q --filter=blob:none --no-checkout --sparse https://github.com/Card-Forge/forge.git "$DIR"
fi
cd "$DIR"
git sparse-checkout set forge-gui/res/cardsfolder forge-game/src/main/java forge-ai/src/main/java forge-core/src/main/java
git fetch -q --depth 1 origin "$COMMIT"
git checkout -q "$COMMIT"
echo "$DIR"
