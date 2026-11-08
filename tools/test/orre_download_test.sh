#!/bin/bash
# ORRE's packs download (src/platform/updater.c Updater_Orre*, the DANCE machine's songs code) end to end on Linux,
# against a local web server that plays the GitHub release. usage: tools/test/orre_download_test.sh [ORRE_DIR]
#   (NOBUILD=1 skips make, OUT=dir; ORRE_DIR: real packs, from tools/orre/make_pack.sh; without them small stand-ins.)
#   A game folder with no orre/: the release's MoemonOrre.zip is found, downloaded and unpacked next to the exe, byte
#   for byte; a damaged download is refused; a game that already has those packs isn't offered them again.
#   Exit code = number of failed checks.
cd "$(dirname "$0")/../.." || exit 99
OUT=${OUT:-/tmp/moemon_orre_download_test}
rm -rf "$OUT"; mkdir -p "$OUT/src" "$OUT/srv"
if [ -z "$NOBUILD" ]; then
    make linux -j"$(nproc)" > "$OUT/build.log" 2>&1 || { echo "build failed, see $OUT/build.log"; exit 99; }
fi
fails=0
check() { local what=$1; shift; if "$@"; then echo "  ok   $what"; else echo "  FAIL $what"; fails=$((fails+1)); fi; }
has() { grep -qF -- "$2" "$1"; }

SRC="${1:-}"
if [ -z "$SRC" ]; then   # stand-ins: the download doesn't read what's inside
    SRC="$OUT/src"
    head -c 3000000 /dev/urandom > "$SRC/orre3d.pak"
    head -c 500000 /dev/urandom > "$SRC/orre_data.pak"
    head -c 100000 /dev/urandom > "$SRC/orre_text.pak"
fi
tools/release/package_orre.sh "$OUT/srv" "$SRC" > /dev/null || { echo "package failed"; exit 99; }
ID=$(cat "$OUT/srv/orre_id.txt")
Z="$OUT/srv/MoemonOrre.zip"
manifest() {   # manifest SHA
    printf 'version 9.9.9\ntag v9.9.9\nchannel main\norre v9.9.9 MoemonOrre.zip %s %s %s\n' "$(stat -c %s "$Z")" "$1" "$ID" \
        > "$OUT/srv/update.txt"
}
PORT=$((20000 + RANDOM % 20000))
python3 tools/test/slow_http.py $PORT "$OUT/srv" 2> "$OUT/http.log" &
P1=$!
trap 'kill $P1 2>/dev/null' EXIT
sleep 1

play() {   # play NAME: a game folder of its own that asks for ORRE's packs and downloads what it's offered
    local G="$OUT/$1"
    mkdir -p "$G"; cp pokeemerald64 "$G/"
    cat > "$G/a.txt" <<'SCRIPT'
pressuntil A field 5000
mp orrecheck
wait 120
mp orrestate
mp orredownload
wait 900
mp orrestate
wait 900
mp orrestate
wait 1200
mp orrestate
wait 1200
mp orrestate
SCRIPT
    (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$G/a.txt" MOEMON_SHOTDIR="$G" \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$G/a.ini" MOEMON_SAVE="$G/a.sav" \
        MOEMON_CONTROLS_CONFIG="$G/a_controls.ini" MOEMON_UPDATE_URL="http://127.0.0.1:$PORT" \
        timeout -k 10 300 ./pokeemerald64 > "$G/log.txt" 2>&1)
}

echo "a damaged download"
manifest 0000000000000000000000000000000000000000000000000000000000000000
play damaged
check "refused" has "$OUT/damaged/log.txt" "orre: THE DOWNLOAD IS DAMAGED (CHECKSUM)"
check "nothing unpacked" test ! -e "$OUT/damaged/orre/orre3d.pak"

echo "the first time"
manifest "$(sha256sum "$Z" | cut -d' ' -f1)"
play first
check "offered" has "$OUT/first/log.txt" "[test] orre state 2 "
check "installed" has "$OUT/first/log.txt" "orre: $ID installed"
check "same files" cmp -s "$OUT/first/orre/orre3d.pak" "$SRC/orre3d.pak"
check "same data" cmp -s "$OUT/first/orre/orre_data.pak" "$SRC/orre_data.pak"
check "the id kept" test "$(cat "$OUT/first/orre/orre_version.txt" 2>/dev/null)" = "$ID"
check "the download removed" test ! -e "$OUT/first/orre_download.zip"

echo "already there"
mkdir -p "$OUT/again"; cp -r "$OUT/first/orre" "$OUT/again/"
play again
check "up to date" has "$OUT/again/log.txt" "[test] orre state 3 "
check "nothing downloaded" bash -c "! grep -q 'orre: downloading' '$OUT/again/log.txt'"
# the boat (src/orre/orre_ferry.c): with real packs only (stand-ins aren't a world): a game with no orre/ sails from
# SLATEPORT with the ORRE TICKET, the clerk offers the download, it's installed, and the boat lands at the OUTSKIRT STAND
if [ -n "$1" ]; then
    echo "the boat"
    manifest "$(sha256sum "$Z" | cut -d' ' -f1)"
    G="$OUT/boat"; mkdir -p "$G"; cp pokeemerald64 "$G/"
    cat > "$G/a.txt" <<'SCRIPT'
pressuntil A field 5000
mp setflag 0x864
mp additem 745 1
mp additem 810 1
mp warp 9,9,8,11
wait 80
mp face up
wait 10
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
press A 2
wait 150
press UP 2
wait 10
press A 2
waitlog 3000 orre: the boat checked
wait 60
press A 2
wait 60
press A 2
wait 60
press A 2
waitlog 9000 ORRE's packs downloaded
pressuntillog A 1500 orre: sailing from
waitlog 3000 voyage: arrived
wait 60
mp where
SCRIPT
    (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$G/a.txt" MOEMON_SHOTDIR="$G" \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$G/a.ini" MOEMON_SAVE="$G/a.sav" \
        MOEMON_CONTROLS_CONFIG="$G/a_controls.ini" MOEMON_UPDATE_URL="http://127.0.0.1:$PORT" \
        timeout -k 10 400 ./pokeemerald64 > "$G/log.txt" 2>&1)
    check "offered at the boat" has "$G/log.txt" "orre: the boat checked ORRE's packs: 2"
    check "downloaded" has "$G/log.txt" "orre: ORRE's packs downloaded: 1"
    check "sailed" has "$G/log.txt" "[test] where map 45.0 xy 33,40"
fi
# at launch (updater.c OrrePreWorker): a release folder with no orre/ and no game update waiting fetches ORRE's packs
# in the background by itself; with the release unreachable it only logs it (next launch)
echo "at launch"
VER=$(grep -o 'MOEMON_VERSION "[0-9.]*"' include/mp_version.h | head -1 | cut -d'"' -f2)
printf 'version %s\ntag v%s\nchannel main\norre v%s MoemonOrre.zip %s %s %s\n' "$VER" "$VER" "$VER" "$(stat -c %s "$Z")" \
    "$(sha256sum "$Z" | cut -d' ' -f1)" "$ID" > "$OUT/srv/update.txt"
for case in launch offline; do
    G="$OUT/$case"; mkdir -p "$G"; cp pokeemerald64 "$G/"; echo release > "$G/moemon_release.txt"
    printf 'pressuntil A field 5000\nwait 1800\nquit\n' > "$G/a.txt"
    URL="http://127.0.0.1:$PORT"; [ "$case" = offline ] && URL="http://127.0.0.1:1"
    (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$G/a.txt" MOEMON_SHOTDIR="$G" \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$G/a.ini" MOEMON_SAVE="$G/a.sav" \
        MOEMON_CONTROLS_CONFIG="$G/a_controls.ini" MOEMON_UPDATE_URL="$URL" \
        timeout -k 10 300 ./pokeemerald64 > "$G/log.txt" 2>&1)
done
check "fetched at launch" has "$OUT/launch/log.txt" "orre: fetching ORRE's packs in the background"
check "installed at launch" cmp -s "$OUT/launch/orre/orre3d.pak" "$SRC/orre3d.pak"
check "offline: skipped" bash -c "! test -e '$OUT/offline/orre/orre3d.pak' && ! grep -q 'crash' '$OUT/offline/log.txt'"
echo "$fails failed"
exit $fails
