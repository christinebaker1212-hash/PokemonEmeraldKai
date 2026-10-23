#!/bin/bash
# The auto-updater (src/platform/updater.c) end to end, on Linux, against a local web server that
# plays the GitHub release. usage: tools/test/update_test.sh   (NOBUILD=1 skips make, OUT=dir)
#   WIN=1: the Windows build under Wine instead (needs pokeemerald64.exe, see tools/release/build_windows.sh,
#   and SDL2_DLL or ./SDL2/x86_64-w64-mingw32/bin/SDL2.dll).
#   Each case unpacks the current release zip into a fresh folder with a player's save and .ini, then
#   starts the game with MOEMON_UPDATE_URL pointing at the server. Cases: offline, up to date, damaged
#   download, ESC during the download, a failure halfway through installing (rollback), a real update
#   (install, keep the save and .ini, restart into the new files, which clears the old ones), another
#   start, and a folder that isn't a release (no check at all).
# Exit code = number of failed checks.
cd "$(dirname "$0")/../.." || exit 99
REPO=$(pwd)
OUT=${OUT:-/tmp/moemon_update_test}
rm -rf "$OUT"; mkdir -p "$OUT"
if [ -n "$WIN" ]; then
    PLAT=windows; EXE=MoemonEmeraldKaiCoop.exe; NOBUILD=1
    export WINEDEBUG=-all WINEPREFIX="${WINEPREFIX:-$OUT/../wineprefix}"
    command -v wine > /dev/null || { echo "wine not installed"; exit 99; }
else
    PLAT=linux; EXE=MoemonEmeraldKaiCoop
fi
W() { if [ -n "$WIN" ]; then echo "Z:$1"; else echo "$1"; fi; }   # a path as the game sees it
if [ -z "$NOBUILD" ]; then
    make linux -j"$(nproc)" > "$OUT/build.log" 2>&1 || { echo "build failed, see $OUT/build.log"; exit 99; }
fi
VER=$(sed -n 's/^#define MOEMON_VERSION "\(.*\)"/\1/p' include/mp_version.h)
fails=0
check() {   # check "what" command...
    local what=$1; shift
    if "$@"; then echo "  ok   $what"; else echo "  FAIL $what"; fails=$((fails+1)); fi
}
has() { grep -qF -- "$2" "$1"; }

# the release the player has, and the newer one on the "server"
tools/release/package.sh $PLAT "$OUT/rel" > /dev/null || { echo "package failed"; exit 99; }
OLDZIP="$OUT/rel/MoemonEmeraldKaiCoop-$PLAT.zip"
mkdir -p "$OUT/new" "$OUT/srv"
(cd "$OUT/new" && unzip -q "$OLDZIP")
N="$OUT/new/MoemonEmeraldKaiCoop"
echo "NEW VERSION TEST LINE" >> "$N/README.txt"
echo "added by the update" > "$N/newfile.txt"
mkdir -p "$N/extras"; echo "in a new folder" > "$N/extras/deep.txt"
printf 'AutoUpdate = 1\n# the zip must never replace the player'"'"'s ini\n' > "$N/moemon_coop.ini"
printf 'Moemon Emerald Kai Co-op 9.9.9 (%s)\n' $PLAT > "$N/moemon_release.txt"
(cd "$OUT/new" && zip -qrX "$OUT/srv/MoemonEmeraldKaiCoop-$PLAT.zip" MoemonEmeraldKaiCoop)
Z="$OUT/srv/MoemonEmeraldKaiCoop-$PLAT.zip"
manifest() {   # manifest VERSION SHA
    printf 'version %s\ntag v%s\n%s MoemonEmeraldKaiCoop-%s.zip %s %s %s\n' \
        "$1" "$1" $PLAT $PLAT "$(stat -c %s "$Z")" "$2" $EXE > "$OUT/srv/update.txt"
}
GOODSHA=$(sha256sum "$Z" | cut -d' ' -f1)

PORT=$((20000 + RANDOM % 20000))
SLOWPORT=$((PORT + 1))
python3 tools/test/slow_http.py $PORT "$OUT/srv" 2> "$OUT/http.log" &
P1=$!
python3 tools/test/slow_http.py $SLOWPORT "$OUT/srv" 0.02 2> "$OUT/http_slow.log" &
P2=$!
trap 'kill $P1 $P2 2>/dev/null' EXIT
sleep 1

printf 'wait 30\nlog reached the game\nshot reached\nquit\n' > "$OUT/script.txt"

fresh() {   # a player's game folder: the old release, a save and an ini
    G="$OUT/case_$1"
    rm -rf "$G"; mkdir -p "$G"
    (cd "$G" && unzip -q "$OLDZIP")
    G="$G/MoemonEmeraldKaiCoop"
    head -c 131072 /dev/urandom > "$G/pokeemerald.sav"
    printf 'PlayerName = TESTER\nAutoUpdate = 1\n' > "$G/moemon_coop.ini"
    (cd "$G" && find . -type f -print0 | sort -z | xargs -0 sha256sum) > "$OUT/before_$1.txt"
}
unchanged() {   # every file the folder had is as it was (the game may add its own, like the display .ini),
                # and nothing of the update is left
    (cd "$G" && sha256sum -c --quiet "$OUT/$1" > /dev/null 2>&1) && [ -z "$(find "$G" -name '*.upd_*' -o -name 'update_*')" ]
}
run() {   # run CASE [env...]: the game in $G, log in $G/log.txt, "reached" screenshot in $G/shots
    local c=$1; shift
    rm -rf "$G/shots"; mkdir -p "$G/shots"
    if [ -n "$WIN" ]; then
        # a restarted Windows game is a process of its own: wait for every Wine process to end
        (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$(W "$OUT/script.txt")" \
            MOEMON_SHOTDIR="$(W "$G/shots")" "$@" timeout -k 5 90 wine ./$EXE > log.txt 2>&1; timeout 90 wineserver -w)
    else
        (cd "$G" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$OUT/script.txt" \
            MOEMON_SHOTDIR="$G/shots" "$@" timeout -k 5 90 ./$EXE > log.txt 2>&1)
    fi
    cp "$G/log.txt" "$OUT/log_$c.txt"
}
reached() { ls "$G"/shots/reached* > /dev/null 2>&1; }

echo "1. offline"
fresh 1; run 1 MOEMON_UPDATE_URL=http://127.0.0.1:1/
check "starts the game" reached
check "says there's no update information" has "$G/log.txt" "no update information"
check "nothing changed" unchanged before_1.txt

echo "2. up to date"
manifest "$VER" "$GOODSHA"
fresh 2; run 2 MOEMON_UPDATE_URL=http://127.0.0.1:$PORT/
check "up to date" has "$G/log.txt" "up to date (this is $VER"
check "starts the game" reached
check "nothing changed" unchanged before_2.txt

echo "3. damaged download"
manifest 9.9.9 0000000000000000000000000000000000000000000000000000000000000000
fresh 3; run 3 MOEMON_UPDATE_URL=http://127.0.0.1:$PORT/ MOEMON_UPDATE_TEST_SHOT="$(W "$OUT/damaged")"
check "refuses it" has "$G/log.txt" "THE DOWNLOAD IS DAMAGED (CHECKSUM)"
check "starts this version" reached
check "nothing changed, download removed" unchanged before_3.txt

echo "4. ESC during the download"
manifest 9.9.9 "$GOODSHA"
fresh 4; run 4 MOEMON_UPDATE_URL=http://127.0.0.1:$SLOWPORT/ MOEMON_UPDATE_TEST_ESC=1 MOEMON_UPDATE_TEST_SHOT="$(W "$OUT/skip")"
check "skipped" has "$G/log.txt" "skipped by the player"
check "starts this version" reached
check "nothing changed, download removed" unchanged before_4.txt
check "progress screen saved" test -s "$OUT/skip_download.bmp"

echo "5. failure halfway through installing (rollback)"
fresh 5; run 5 MOEMON_UPDATE_URL=http://127.0.0.1:$PORT/ MOEMON_UPDATE_TEST_FAIL_SWAP=4
check "reports it" has "$G/log.txt" "(TEST)"
check "starts this version" reached
check "everything put back" unchanged before_5.txt
check "no leftovers" test -z "$(find "$G" -name '*.upd_*')"

echo "6. update"
fresh 6; run 6 MOEMON_UPDATE_URL=http://127.0.0.1:$PORT/ MOEMON_UPDATE_TEST_SHOT="$(W "$OUT/update")"
check "downloads it" has "$G/log.txt" "version 9.9.9 is out (this is $VER)"
check "installs it" has "$G/log.txt" "installed version 9.9.9"
check "keeps the player's ini" has "$G/log.txt" "keeping moemon_coop.ini"
check "starts the new exe" has "$G/log.txt" "[update] starting"
check "the restarted game runs" reached
check "new README" has "$G/README.txt" "NEW VERSION TEST LINE"
check "new file" test -f "$G/newfile.txt"
check "new folder" test -f "$G/extras/deep.txt"
check "release marker updated" has "$G/moemon_release.txt" "9.9.9"
check "ini untouched" cmp -s "$G/moemon_coop.ini" <(printf 'PlayerName = TESTER\nAutoUpdate = 1\n')
check "save untouched" bash -c "cd '$G' && grep ' ./pokeemerald.sav\$' '$OUT/before_6.txt' | sha256sum -c --quiet"
check "exe in place" test -f "$G/$EXE"
# the restarted game deletes them; on Windows the old exe can still be closing then, so it may only be
# listed for the start after (case 7 checks that nothing is left)
check "old files cleared or listed for the next start" bash -c "[ -z \"\$(find '$G' -name '*.upd_old*')\" ] || [ -s '$G/update_cleanup.txt' ]"
check "download removed" test ! -e "$G/update_download.zip"
check "no .upd_new left" test -z "$(find "$G" -name '*.upd_new')"
check "result screen saved" test -s "$OUT/update_result.bmp"

echo "7. the next start finds nothing left over"
run 7 MOEMON_NO_UPDATE=1
check "old files gone" test -z "$(find "$G" -name '*.upd_old*')"
check "cleanup list gone" test ! -e "$G/update_cleanup.txt"
check "starts the game" reached

echo "8. not a release folder"
fresh 8; rm "$G/moemon_release.txt"
run 8 MOEMON_UPDATE_URL=http://127.0.0.1:$PORT/
check "no check at all" bash -c "! grep -q 'update information\|is out\|up to date' '$G/log.txt'"
check "starts the game" reached

echo "$fails failed, logs in $OUT"
exit $fails
