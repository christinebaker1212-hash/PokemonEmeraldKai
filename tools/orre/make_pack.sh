#!/bin/bash
# ORRE's packs (docs/ORRE_DECISIONS.md), ripped from Pokemon Colosseum's disc (GC6E01, the one the Colosseum decomp
# matches) into orre/: orre3d.pak (every map's model) and orre_data.pak (text, collision, scripts, floor data, cameras,
# map tables, character models). The game reads only these: no disc. Nothing of the disc or the packs is in the
# repository (orre/ is ignored); the release builds them and ships them.
# usage: tools/orre/make_pack.sh DISC          (a .rvz / .iso / .gcm, or a .zip holding one)
#        ORRE_DISC_URL=URL tools/orre/make_pack.sh   (the release: downloads the disc first)
set -e
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
WORK="${ORRE_WORK:-${XDG_CACHE_HOME:-$HOME/.cache}/moemon/orre}"
mkdir -p "$WORK"
DISC="$1"
if [ -z "$DISC" ]; then
    [ -n "$ORRE_DISC_URL" ] || { echo "usage: $0 DISC  (or ORRE_DISC_URL=...)"; exit 1; }
    DISC="$WORK/disc.download"
    if [ ! -s "$DISC" ]; then
        URL="$ORRE_DISC_URL"
        # a Google Drive share link: its direct download
        if [[ "$URL" =~ drive\.google\.com/file/d/([^/]+) ]]; then
            URL="https://drive.usercontent.google.com/download?id=${BASH_REMATCH[1]}&export=download&confirm=t"
        fi
        curl -sSfL --retry 4 -o "$DISC" "$URL"
    fi
fi
# a zip holding the image
if [ "$(head -c 2 "$DISC")" = "PK" ]; then
    rm -rf "$WORK/disc" && mkdir -p "$WORK/disc"
    python3 -I -c "import zipfile,sys; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "$DISC" "$WORK/disc"
    DISC="$(find "$WORK/disc" -type f \( -iname '*.rvz' -o -iname '*.iso' -o -iname '*.gcm' -o -iname '*.ciso' -o -iname '*.wbfs' \) | head -1)"
    [ -n "$DISC" ] || { echo "no disc image in the zip"; exit 1; }
fi
# decomp-toolkit reads the disc (RVZ too), pinned as the Colosseum decomp pins it
DTK="$WORK/dtk"
DTK_SHA=97c60d797aa6d87093c367d7667c77c03204e91b825b6fdd6361e29468865d76
if [ ! -x "$DTK" ] || ! echo "$DTK_SHA  $DTK" | sha256sum -c --quiet 2>/dev/null; then
    curl -sSfL --retry 4 -o "$DTK" https://github.com/encounter/decomp-toolkit/releases/download/v1.8.3/dtk-linux-x86_64
    echo "$DTK_SHA  $DTK" | sha256sum -c --quiet
    chmod +x "$DTK"
fi
rm -rf "$WORK/files" "$WORK/unpacked"
mkdir -p "$WORK/files"
"$DTK" vfs cp "$DISC:sys/main.dol" "$WORK/files/main.dol" > /dev/null
# the disc the decomp matches: Pokemon Colosseum (USA), GC6E01
echo "870e8b9693ca780782d80f22a6a4572d8ba9458f  $WORK/files/main.dol" | sha1sum -c --quiet \
    || { echo "this isn't Pokemon Colosseum (USA) GC6E01"; exit 1; }
"$DTK" vfs cp "$DISC:files" "$WORK/files/" > /dev/null
python3 -I "$REPO/tools/orre/fsys.py" unpack "$WORK/files" "$WORK/unpacked"
# the music's and sounds' samples aren't in archives: the disc's files/sound/ goes in beside them
mkdir -p "$WORK/unpacked/sound" "$WORK/unpacked/sys"
cp "$WORK/files/sound/"*.samp "$WORK/unpacked/sound/"
cp "$WORK/files/main.dol" "$WORK/unpacked/sys/"   # (its menu tables go in the data pack: build_pack.py DOL_TABLES)
mkdir -p "$REPO/orre"
# (its warnings go to a log; on a failure the log's end is shown: the release's 1.171.2 failed here without a word)
python3 -I "$REPO/tools/orre/build_pack.py" "$WORK/unpacked" "$REPO/orre" 2> "$WORK/build_pack.log" \
    || { echo "build_pack.py failed:"; tail -30 "$WORK/build_pack.log"; exit 1; }
if [ -n "$ORRE_TEST" ]; then
    python3 -I "$REPO/tools/orre/test_pack.py" "$WORK/unpacked" "$REPO/orre" 2> "$WORK/test_pack.log" \
        || { echo "test_pack.py failed:"; tail -30 "$WORK/test_pack.log"; exit 1; }
fi
rm -rf "$WORK/files" "$WORK/disc"   # (the unpacked members stay for the conversions; the disc download too)
ls -la "$REPO/orre"
