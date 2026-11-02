#!/bin/bash
# pret/pokeheartgold's TRAINER CARD graphics (files/a/0/4/9: the card's tiles, its front and back screens and the
# palettes of its colors) at a pinned commit, for tools/sinnoh/build_cards.py (JOHTO's card). Nothing from it is
# committed here. usage: tools/sinnoh/fetch_hgss_card.sh DIR   (DIR/a049.narc)
set -e
DIR="${1:?usage: fetch_hgss_card.sh DIR}"
COMMIT=e1d5a4606640e31c44d36714ad0b5cdd9c8763fd   # (2026-11-02)
if [ -f "$DIR/a049.narc" ] && [ "$(cat "$DIR/.commit" 2>/dev/null)" = "$COMMIT" ]; then
    echo "pokeheartgold's card $COMMIT already in $DIR"
    exit 0
fi
mkdir -p "$DIR"
TMP="$DIR/repo"
rm -rf "$TMP"
git init -q "$TMP"
git -C "$TMP" remote add origin https://github.com/pret/pokeheartgold.git
git -C "$TMP" sparse-checkout set --no-cone /files/a/0/4/9
for i in 1 2 3 4; do
    git -C "$TMP" fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && break
    sleep $((2 ** i))
done
git -C "$TMP" checkout -q FETCH_HEAD
cp "$TMP/files/a/0/4/9" "$DIR/a049.narc"
rm -rf "$TMP"
echo "$COMMIT" > "$DIR/.commit"
echo "pokeheartgold's card $COMMIT in $DIR"
