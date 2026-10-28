#!/bin/bash
# The DANCE machine's songs download (src/platform/updater.c Updater_Songs*, rh_screen.c StateNoSongs) end to end on
# Linux, against a local web server that plays the GitHub release. usage: tools/test/dance_songs_test.sh
#   (NOBUILD=1 skips make, OUT=dir). A game folder with no songs: stepping up to the machine finds the songs on the
#   "release", downloads and unpacks them next to the exe, and the song wheel comes up; then a damaged download is
#   refused, and a game that already has those songs isn't offered them again. Exit code = number of failed checks.
cd "$(dirname "$0")/../.." || exit 99
REPO=$(pwd)
OUT=${OUT:-/tmp/moemon_dance_songs_test}
rm -rf "$OUT"; mkdir -p "$OUT/src/Songs/g01" "$OUT/srv"
if [ -z "$NOBUILD" ]; then
    make linux -j"$(nproc)" > "$OUT/build.log" 2>&1 || { echo "build failed, see $OUT/build.log"; exit 99; }
fi
fails=0
check() { local what=$1; shift; if "$@"; then echo "  ok   $what"; else echo "  FAIL $what"; fails=$((fails+1)); fi; }
has() { grep -qF -- "$2" "$1"; }

# two songs, packed the way the release workflow packs them
for s in "1998" "321 STARS"; do cp -r "$(ls -d stepmania/Songs/*/"$s" | head -1)" "$OUT/src/Songs/g01/"; done
tools/release/package_songs.sh "$OUT/srv" "$OUT/src" > /dev/null || { echo "package failed"; exit 99; }
ID=$(cat "$OUT/srv/songs_id.txt")
Z="$OUT/srv/MoemonDanceSongs.zip"
manifest() {   # manifest SHA
    printf 'version 9.9.9\ntag v9.9.9\nchannel main\nsongs v9.9.9 MoemonDanceSongs.zip %s %s %s\n' "$(stat -c %s "$Z")" "$1" "$ID" \
        > "$OUT/srv/update.txt"
}
PORT=$((20000 + RANDOM % 20000))
python3 tools/test/slow_http.py $PORT "$OUT/srv" 2> "$OUT/http.log" &
P1=$!
trap 'kill $P1 2>/dev/null' EXIT
sleep 1

play() {   # play NAME: a game folder of its own, stepping up to the machine and pressing A on whatever it offers
    local G="$OUT/$1"
    mkdir -p "$G"; cp pokeemerald64 "$G/"
    cat > "$G/a.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
press UP 4
wait 20
press A 2
wait 60
press A 2
wait 90
shot a_offer
press A 2
wait 600
shot b_after
wait 60
SCRIPT
    (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$G/a.txt" MOEMON_SHOTDIR="$G" \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$G/a.ini" MOEMON_SAVE="$G/a.sav" \
        MOEMON_CONTROLS_CONFIG="$G/a_controls.ini" MOEMON_DEBUG_WARP=11,19,1,7 MOEMON_UPDATE_URL="http://127.0.0.1:$PORT" \
        timeout -k 10 120 ./pokeemerald64 > "$G/log.txt" 2>&1)
}

echo "a damaged download"
manifest 0000000000000000000000000000000000000000000000000000000000000000
play damaged
check "refused" has "$OUT/damaged/log.txt" "songs: THE DOWNLOAD IS DAMAGED (CHECKSUM)"
check "nothing unpacked" test ! -e "$OUT/damaged/stepmania/Songs"

echo "the first time at the machine"
manifest "$(sha256sum "$Z" | cut -d' ' -f1)"
play fresh
check "the songs were offered" has "$OUT/fresh/log.txt" "songs: $ID on the release"
check "installed" has "$OUT/fresh/log.txt" "songs: $ID installed"
check "same files" diff -r "$OUT/src/Songs" "$OUT/fresh/stepmania/Songs"
check "the id kept" grep -qx "$ID" "$OUT/fresh/stepmania/songs_version.txt"
check "the download removed" test ! -e "$OUT/fresh/songs_download.zip"
check "the wheel has them" has "$OUT/fresh/log.txt" "dance: 2 songs after the download"

echo "already there"
mkdir -p "$OUT/again"; cp -r "$OUT/fresh/stepmania" "$OUT/again/"
play again
check "up to date" grep -q "songs: $ID on the release ([0-9]* bytes), $ID here" "$OUT/again/log.txt"
check "nothing downloaded" bash -c "! grep -q 'songs: downloading' '$OUT/again/log.txt'"
command -v convert > /dev/null && for f in "$OUT"/*/*.bmp; do convert "$f" -scale 200% "${f%.bmp}.png" && rm -f "$f"; done
echo "$fails failed"
exit $fails
